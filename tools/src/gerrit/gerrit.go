// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Package gerrit provides helpers for obtaining information from Tint's gerrit instance
package gerrit

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net/url"
	"strconv"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/container"
	"github.com/andygrunwald/go-gerrit"
	"go.chromium.org/luci/auth"
)

// Gerrit is the interface to gerrit
type Gerrit struct {
	client        *gerrit.Client
	authenticated bool
}

// Patchset refers to a single gerrit patchset
type Patchset struct {
	// Gerrit host
	Host string
	// Gerrit project
	Project string
	// Change ID
	Change int
	// Patchset ID
	Patchset int
}

// ChangeInfo is an alias to gerrit.ChangeInfo
type ChangeInfo = gerrit.ChangeInfo

// LatestPatchset returns the latest Patchset from the ChangeInfo
func LatestPatchset(change *ChangeInfo) Patchset {
	u, _ := url.Parse(change.URL)
	ps := Patchset{
		Host:     u.Host,
		Project:  change.Project,
		Change:   change.Number,
		Patchset: change.Revisions[change.CurrentRevision].Number,
	}
	return ps
}

// RegisterFlags registers the command line flags to populate p
func (p *Patchset) RegisterFlags(defaultHost, defaultProject string) {
	flag.StringVar(&p.Host, "host", defaultHost, "gerrit host")
	flag.StringVar(&p.Project, "project", defaultProject, "gerrit project")
	flag.IntVar(&p.Change, "cl", 0, "gerrit change id")
	flag.IntVar(&p.Patchset, "ps", 0, "gerrit patchset id")
}

// RefsChanges returns the gerrit 'refs/changes/X/Y/Z' string for the patchset
func (p Patchset) RefsChanges() string {
	// https://gerrit-review.googlesource.com/Documentation/intro-user.html
	// A change ref has the format refs/changes/X/Y/Z where X is the last two
	// digits of the change number, Y is the entire change number, and Z is the
	// patch set. For example, if the change number is 263270, the ref would be
	// refs/changes/70/263270/2 for the second patch set.
	shortChange := fmt.Sprintf("%.2v", p.Change)
	shortChange = shortChange[len(shortChange)-2:]
	return fmt.Sprintf("refs/changes/%v/%v/%v", shortChange, p.Change, p.Patchset)
}

// New returns a new Gerrit instance. If credentials are not provided, then
// New() will automatically attempt to load them from the gitcookies file.
func New(ctx context.Context, opts auth.Options, url string) (*Gerrit, error) {
	http, err := auth.NewAuthenticator(ctx, auth.InteractiveLogin, opts).Client()
	if err != nil {
		return nil, fmt.Errorf("couldn't create gerrit client: %w", err)
	}

	client, err := gerrit.NewClient(url, http)
	if err != nil {
		return nil, fmt.Errorf("couldn't create gerrit client: %w", err)
	}

	return &Gerrit{client, true}, nil
}

// QueryExtraData holds extra data to query for with QueryChangesWith()
type QueryExtraData struct {
	Labels           bool
	Messages         bool
	CurrentRevision  bool
	DetailedAccounts bool
	Submittable      bool
}

// QueryChanges returns the changes that match the given query strings.
// See: https://gerrit-review.googlesource.com/Documentation/user-search.html#search-operators
func (g *Gerrit) QueryChangesWith(extras QueryExtraData, queries ...string) (changes []gerrit.ChangeInfo, query string, err error) {
	changes = []gerrit.ChangeInfo{}
	query = strings.Join(queries, "+")

	changeOpts := gerrit.ChangeOptions{}
	if extras.Labels {
		changeOpts.AdditionalFields = append(changeOpts.AdditionalFields, "LABELS")
	}
	if extras.Messages {
		changeOpts.AdditionalFields = append(changeOpts.AdditionalFields, "MESSAGES")
	}
	if extras.CurrentRevision {
		changeOpts.AdditionalFields = append(changeOpts.AdditionalFields, "CURRENT_REVISION")
	}
	if extras.DetailedAccounts {
		changeOpts.AdditionalFields = append(changeOpts.AdditionalFields, "DETAILED_ACCOUNTS")
	}
	if extras.Submittable {
		changeOpts.AdditionalFields = append(changeOpts.AdditionalFields, "SUBMITTABLE")
	}

	for {
		batch, _, err := g.client.Changes.QueryChanges(&gerrit.QueryChangeOptions{
			QueryOptions:  gerrit.QueryOptions{Query: []string{query}},
			Skip:          len(changes),
			ChangeOptions: changeOpts,
		})
		if err != nil {
			return nil, "", err
		}

		changes = append(changes, *batch...)
		if len(*batch) == 0 || !(*batch)[len(*batch)-1].MoreChanges {
			break
		}
	}
	return changes, query, nil
}

// QueryChanges returns the changes that match the given query strings.
// See: https://gerrit-review.googlesource.com/Documentation/user-search.html#search-operators
func (g *Gerrit) QueryChanges(queries ...string) (changes []gerrit.ChangeInfo, query string, err error) {
	return g.QueryChangesWith(QueryExtraData{}, queries...)
}

// ChangesSubmittedTogether returns the changes that want to be submitted together
// See: https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#submitted-together
func (g *Gerrit) ChangesSubmittedTogether(changeID string) (changes []gerrit.ChangeInfo, err error) {
	info, _, err := g.client.Changes.ChangesSubmittedTogether(changeID)
	if err != nil {
		return nil, err
	}
	return *info, nil
}

func (g *Gerrit) AddLabel(changeID, revisionID, message, label string, value int) error {
	_, _, err := g.client.Changes.SetReview(changeID, revisionID, &gerrit.ReviewInput{
		Message: message,
		Labels:  map[string]string{label: fmt.Sprint(value)},
	})
	if err != nil {
		return err
	}
	return nil
}

// Abandon abandons the change with the given changeID.
func (g *Gerrit) Abandon(changeID string) error {
	_, _, err := g.client.Changes.AbandonChange(changeID, &gerrit.AbandonInput{})
	if err != nil {
		return err
	}
	return nil
}

// CreateChange creates a new change in the given project and branch, with the
// given subject. If wip is true, then the change is constructed as
// Work-In-Progress.
func (g *Gerrit) CreateChange(project, branch, subject string, wip bool) (*ChangeInfo, error) {
	change, _, err := g.client.Changes.CreateChange(&gerrit.ChangeInput{
		Project:        project,
		Branch:         branch,
		Subject:        subject,
		WorkInProgress: wip,
	})
	if err != nil {
		return nil, err
	}
	return change, nil
}

// EditFiles replaces the content of the files in the given change. It deletes deletedFiles.
// If newCommitMsg is not an empty string, then the commit message is replaced
// with the string value.
func (g *Gerrit) EditFiles(changeID, newCommitMsg string, files map[string]string, deletedFiles []string) (Patchset, error) {
	if newCommitMsg != "" {
		resp, err := g.client.Changes.ChangeCommitMessageInChangeEdit(changeID, &gerrit.ChangeEditMessageInput{
			Message: newCommitMsg,
		})
		if err != nil && resp.StatusCode != 409 { // 409 no changes were made
			return Patchset{}, err
		}
	}
	for path, content := range files {
		resp, err := g.client.Changes.ChangeFileContentInChangeEdit(changeID, path, content)
		if err != nil && resp.StatusCode != 409 { // 409 no changes were made
			return Patchset{}, err
		}
	}
	for _, path := range deletedFiles {
		resp, err := g.client.Changes.DeleteFileInChangeEdit(changeID, path)
		if err != nil && resp.StatusCode != 409 { // 409 no changes were made
			return Patchset{}, err
		}
	}

	resp, err := g.client.Changes.PublishChangeEdit(changeID, "NONE")
	if err != nil && resp.StatusCode != 409 { // 409 no changes were made
		return Patchset{}, err
	}

	return g.LatestPatchset(changeID)
}

// LatestPatchset returns the latest patchset for the change.
func (g *Gerrit) LatestPatchset(changeID string) (Patchset, error) {
	change, _, err := g.client.Changes.GetChange(changeID, &gerrit.ChangeOptions{
		AdditionalFields: []string{"CURRENT_REVISION"},
	})
	if err != nil {
		return Patchset{}, err
	}
	ps := Patchset{
		Host:     g.client.BaseURL().Host,
		Project:  change.Project,
		Change:   change.Number,
		Patchset: change.Revisions[change.CurrentRevision].Number,
	}
	return ps, nil
}

// AddHashtags adds the given hashtags to the change
func (g *Gerrit) AddHashtags(changeID string, tags container.Set[string]) error {
	_, resp, err := g.client.Changes.SetHashtags(changeID, &gerrit.HashtagsInput{
		Add: tags.List(),
	})
	if err != nil && resp.StatusCode != 409 { // 409: already ready
		return err
	}
	return nil
}

// CommentSide is an enumerator for specifying which side code-comments should
// be shown.
type CommentSide int

const (
	// Left is used to specifiy that code comments should appear on the parent
	// change
	Left CommentSide = iota
	// Right is used to specifiy that code comments should appear on the new
	// change
	Right
)

// FileComment describes a single comment on a file
type FileComment struct {
	Path    string      // The file path
	Side    CommentSide // Which side the comment should appear
	Line    int         // The 1-based line number for the comment
	Message string      // The comment message
}

// Comment posts a review comment on the given patchset.
// If comments is an optional list of file-comments to include in the comment.
func (g *Gerrit) Comment(ps Patchset, msg string, comments []FileComment) error {
	input := &gerrit.ReviewInput{
		Message: msg,
	}
	if len(comments) > 0 {
		input.Comments = map[string][]gerrit.CommentInput{}
		for _, c := range comments {
			ci := gerrit.CommentInput{
				Line: c.Line,
				// Updated: &gerrit.Timestamp{Time: time.Now()},
				Message: c.Message,
			}
			if c.Side == Left {
				ci.Side = "PARENT"
			} else {
				ci.Side = "REVISION"
			}
			input.Comments[c.Path] = append(input.Comments[c.Path], ci)
		}
	}
	_, _, err := g.client.Changes.SetReview(strconv.Itoa(ps.Change), strconv.Itoa(ps.Patchset), input)
	if err != nil {
		return err
	}
	return nil
}

// SetReadyForReview marks the change as ready for review.
func (g *Gerrit) SetReadyForReview(changeID, message, reviewer string) error {
	resp, err := g.client.Changes.SetReadyForReview(changeID, &gerrit.ReadyForReviewInput{
		Message: message,
	})
	if err != nil && resp.StatusCode != 409 { // 409: already ready
		return err
	}
	if reviewer != "" {
		// Log the reviewer and then replace with enga@.
		// TODO(crbug.com/dawn/1940): Use the actual reviewer when the bot is stable.
		log.Printf("Got reviewer %s", reviewer)
		reviewer = "enga@chromium.org"
		_, resp, err = g.client.Changes.AddReviewer(changeID, &gerrit.ReviewerInput{
			Reviewer: reviewer,
		})
		if err != nil && resp.StatusCode != 409 { // 409: already ready
			return err
		}
	}
	return nil
}
