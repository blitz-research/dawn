// Copyright 2020 The Dawn Authors
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

#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

// Clear the content of the result buffer into 0xFFFFFFFF.
constexpr static uint64_t kSentinelValue = ~uint64_t(0u);
constexpr static uint64_t kZero = 0u;
constexpr static unsigned int kRTSize = 4;
constexpr uint64_t kMinDestinationOffset = 256;
constexpr uint64_t kMinCount = kMinDestinationOffset / sizeof(uint64_t);
constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
constexpr wgpu::TextureFormat kDepthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;

class QueryTests : public DawnTest {
  protected:
    wgpu::Buffer CreateResolveBuffer(uint64_t size) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = wgpu::BufferUsage::QueryResolve | wgpu::BufferUsage::CopySrc |
                           wgpu::BufferUsage::CopyDst;
        return device.CreateBuffer(&descriptor);
    }

    wgpu::Texture CreateRenderTexture(wgpu::TextureFormat format) {
        wgpu::TextureDescriptor descriptor;
        descriptor.size = {kRTSize, kRTSize, 1};
        descriptor.format = format;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment;
        return device.CreateTexture(&descriptor);
    }
};

class OcclusionExpectation : public detail::Expectation {
  public:
    enum class Result { Zero, NonZero };

    ~OcclusionExpectation() override = default;

    explicit OcclusionExpectation(Result expected) { mExpected = expected; }

    testing::AssertionResult Check(const void* data, size_t size) override {
        ASSERT(size % sizeof(uint64_t) == 0);
        const uint64_t* actual = static_cast<const uint64_t*>(data);
        for (size_t i = 0; i < size / sizeof(uint64_t); i++) {
            if (actual[i] == kSentinelValue) {
                return testing::AssertionFailure()
                       << "Data[" << i << "] was not written (it kept the sentinel value of "
                       << kSentinelValue << ")." << std::endl;
            }
            if (mExpected == Result::Zero && actual[i] != 0) {
                return testing::AssertionFailure()
                       << "Expected data[" << i << "] to be zero, actual: " << actual[i] << "."
                       << std::endl;
            }
            if (mExpected == Result::NonZero && actual[i] == 0) {
                return testing::AssertionFailure()
                       << "Expected data[" << i << "] to be non-zero." << std::endl;
            }
        }

        return testing::AssertionSuccess();
    }

  private:
    Result mExpected;
};

class OcclusionQueryTests : public QueryTests {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Create basic render pipeline
        vsModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f( 1.0,  1.0),
                    vec2f(-1.0, -1.0),
                    vec2f( 1.0, -1.0));
                return vec4f(pos[VertexIndex], 0.0, 1.0);
            })");

        fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;

        pipeline = device.CreateRenderPipeline(&descriptor);
    }

    struct ScissorRect {
        uint32_t x;
        uint32_t y;
        uint32_t width;
        uint32_t height;
    };

    wgpu::QuerySet CreateOcclusionQuerySet(uint32_t count) {
        wgpu::QuerySetDescriptor descriptor;
        descriptor.count = count;
        descriptor.type = wgpu::QueryType::Occlusion;
        return device.CreateQuerySet(&descriptor);
    }

    void TestOcclusionQueryWithDepthStencilTest(bool depthTestEnabled,
                                                bool stencilTestEnabled,
                                                OcclusionExpectation::Result expected) {
        constexpr uint32_t kQueryCount = 1;

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;

        // Enable depth and stencil tests and set comparison tests never pass.
        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil(kDepthStencilFormat);
        depthStencil->depthCompare =
            depthTestEnabled ? wgpu::CompareFunction::Never : wgpu::CompareFunction::Always;
        depthStencil->stencilFront.compare =
            stencilTestEnabled ? wgpu::CompareFunction::Never : wgpu::CompareFunction::Always;
        depthStencil->stencilBack.compare =
            stencilTestEnabled ? wgpu::CompareFunction::Never : wgpu::CompareFunction::Always;

        wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&descriptor);

        wgpu::Texture renderTarget = CreateRenderTexture(kColorFormat);
        wgpu::TextureView renderTargetView = renderTarget.CreateView();

        wgpu::Texture depthTexture = CreateRenderTexture(kDepthStencilFormat);
        wgpu::TextureView depthTextureView = depthTexture.CreateView();

        wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
        // Set all bits in buffer to check 0 is correctly written if there is no sample passed the
        // occlusion testing
        queue.WriteBuffer(destination, 0, &kSentinelValue, sizeof(kSentinelValue));

        utils::ComboRenderPassDescriptor renderPass({renderTargetView}, depthTextureView);
        renderPass.occlusionQuerySet = querySet;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(renderPipeline);
        pass.SetStencilReference(0);
        pass.BeginOcclusionQuery(0);
        pass.Draw(3);
        pass.EndOcclusionQuery();
        pass.End();

        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, sizeof(uint64_t), new OcclusionExpectation(expected));
    }

    void TestOcclusionQueryWithScissorTest(ScissorRect rect,
                                           OcclusionExpectation::Result expected) {
        constexpr uint32_t kQueryCount = 1;

        wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
        // Set all bits in buffer to check 0 is correctly written if there is no sample passed the
        // occlusion testing
        queue.WriteBuffer(destination, 0, &kSentinelValue, sizeof(kSentinelValue));

        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        renderPass.renderPassInfo.occlusionQuerySet = querySet;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetScissorRect(rect.x, rect.y, rect.width, rect.height);
        pass.BeginOcclusionQuery(0);
        pass.Draw(3);
        pass.EndOcclusionQuery();
        pass.End();

        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, sizeof(uint64_t), new OcclusionExpectation(expected));
    }

    wgpu::ShaderModule vsModule;
    wgpu::ShaderModule fsModule;

    wgpu::RenderPipeline pipeline;
};

// Test creating query set with the type of Occlusion
TEST_P(OcclusionQueryTests, QuerySetCreation) {
    // Zero-sized query set is allowed.
    CreateOcclusionQuerySet(0);

    CreateOcclusionQuerySet(1);
}

// Test destroying query set
TEST_P(OcclusionQueryTests, QuerySetDestroy) {
    wgpu::QuerySet querySet = CreateOcclusionQuerySet(1);
    querySet.Destroy();
}

// Draw a bottom right triangle with depth/stencil testing enabled and check whether there is
// sample passed the testing by non-precise occlusion query with the results:
// zero indicates that no sample passed depth/stencil testing,
// non-zero indicates that at least one sample passed depth/stencil testing.
TEST_P(OcclusionQueryTests, QueryWithDepthStencilTest) {
    // Disable depth/stencil testing, the samples always pass the testing, the expected occlusion
    // result is non-zero.
    TestOcclusionQueryWithDepthStencilTest(false, false, OcclusionExpectation::Result::NonZero);

    // Only enable depth testing and set the samples never pass the testing, the expected occlusion
    // result is zero.
    TestOcclusionQueryWithDepthStencilTest(true, false, OcclusionExpectation::Result::Zero);

    // Only enable stencil testing and set the samples never pass the testing, the expected
    // occlusion result is zero.
    TestOcclusionQueryWithDepthStencilTest(false, true, OcclusionExpectation::Result::Zero);
}

// Draw a bottom right triangle with scissor testing enabled and check whether there is
// sample passed the testing by non-precise occlusion query with the results:
// zero indicates that no sample passed scissor testing,
// non-zero indicates that at least one sample passed scissor testing.
TEST_P(OcclusionQueryTests, QueryWithScissorTest) {
    // Test there are samples passed scissor testing, the expected occlusion result is non-zero.
    TestOcclusionQueryWithScissorTest({2, 1, 2, 1}, OcclusionExpectation::Result::NonZero);

    // Test there is no sample passed scissor testing, the expected occlusion result is zero.
    TestOcclusionQueryWithScissorTest({0, 0, 2, 1}, OcclusionExpectation::Result::Zero);
}

// Test begin occlusion query with same query index on different render pass
TEST_P(OcclusionQueryTests, Rewrite) {
    constexpr uint32_t kQueryCount = 1;

    wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    // Set all bits in buffer to check 0 is correctly written if there is no sample passed the
    // occlusion testing
    queue.WriteBuffer(destination, 0, &kSentinelValue, sizeof(kSentinelValue));

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    renderPass.renderPassInfo.occlusionQuerySet = querySet;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Begin occlusion without draw call
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.BeginOcclusionQuery(0);
    pass.EndOcclusionQuery();
    pass.End();

    // Begin occlusion with same query index with draw call
    wgpu::RenderPassEncoder rewritePass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    rewritePass.SetPipeline(pipeline);
    rewritePass.BeginOcclusionQuery(0);
    rewritePass.Draw(3);
    rewritePass.EndOcclusionQuery();
    rewritePass.End();

    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER(destination, 0, sizeof(uint64_t),
                  new OcclusionExpectation(OcclusionExpectation::Result::NonZero));
}

// Test resolving occlusion query correctly if the queries are written sparsely, which also tests
// the query resetting at the start of render passes on Vulkan backend.
TEST_P(OcclusionQueryTests, ResolveSparseQueries) {
    constexpr uint32_t kQueryCount = 7;

    wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    // Set sentinel values to check the queries are resolved correctly if the queries are
    // written sparsely.
    std::vector<uint64_t> sentinelValues(kQueryCount, kSentinelValue);
    queue.WriteBuffer(destination, 0, sentinelValues.data(), kQueryCount * sizeof(uint64_t));

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    renderPass.renderPassInfo.occlusionQuerySet = querySet;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);

    // Write queries sparsely for testing the query resetting on Vulkan and resolving unwritten
    // queries to 0.
    // 0 - not written (tests starting with not written).
    // 1 - written (tests combing multiple written, although other tests already do it).
    // 2 - written.
    // 3 - not written (tests skipping over not written in the middle).
    // 4 - not written.
    // 5 - written (tests another written query in the middle).
    // 6 - not written (tests the last query not being written).
    pass.BeginOcclusionQuery(1);
    pass.Draw(3);
    pass.EndOcclusionQuery();
    pass.BeginOcclusionQuery(2);
    pass.Draw(3);
    pass.EndOcclusionQuery();
    pass.BeginOcclusionQuery(5);
    pass.Draw(3);
    pass.EndOcclusionQuery();
    pass.End();

    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The query at index 0 should be resolved to 0.
    EXPECT_BUFFER_U64_RANGE_EQ(&kZero, destination, 0, 1);
    EXPECT_BUFFER(destination, sizeof(uint64_t), 2 * sizeof(uint64_t),
                  new OcclusionExpectation(OcclusionExpectation::Result::NonZero));
    // The queries at index 3 and 4 should be resolved to 0.
    std::vector<uint64_t> zeros(2, kZero);
    EXPECT_BUFFER_U64_RANGE_EQ(zeros.data(), destination, 3 * sizeof(uint64_t), 2);
    EXPECT_BUFFER(destination, 5 * sizeof(uint64_t), sizeof(uint64_t),
                  new OcclusionExpectation(OcclusionExpectation::Result::NonZero));
    // The query at index 6 should be resolved to 0.
    EXPECT_BUFFER_U64_RANGE_EQ(&kZero, destination, 6 * sizeof(uint64_t), 1);
}

// Test resolving occlusion query to 0 if all queries are not written
TEST_P(OcclusionQueryTests, ResolveWithoutWritten) {
    constexpr uint32_t kQueryCount = 1;

    wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    // Set sentinel values to check 0 is correctly written if resolving query set without
    // any written.
    queue.WriteBuffer(destination, 0, &kSentinelValue, sizeof(kSentinelValue));

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U64_RANGE_EQ(&kZero, destination, 0, 1);
}

// Test setting an occlusion query to non-zero, then rewriting it without drawing, resolves to 0.
TEST_P(OcclusionQueryTests, RewriteNoDrawToZero) {
    constexpr uint32_t kQueryCount = 1;

    wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    // Set all bits in buffer to check 0 is correctly written if there is no sample passed the
    // occlusion testing
    queue.WriteBuffer(destination, 0, &kSentinelValue, sizeof(kSentinelValue));

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    renderPass.renderPassInfo.occlusionQuerySet = querySet;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Do an occlusion query with a draw call
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.BeginOcclusionQuery(0);
    pass.Draw(3);
    pass.EndOcclusionQuery();
    pass.End();

    // Do another occlusion query with no draw calls, rewriting the same index.
    wgpu::RenderPassEncoder rewritePass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    rewritePass.BeginOcclusionQuery(0);
    rewritePass.EndOcclusionQuery();
    rewritePass.End();

    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER(destination, 0, sizeof(uint64_t),
                  new OcclusionExpectation(OcclusionExpectation::Result::Zero));
}

// Test setting an occlusion query to non-zero, then rewriting it without drawing, resolves to 0.
// Do the two queries+resolves in separate submits.
TEST_P(OcclusionQueryTests, RewriteNoDrawToZeroSeparateSubmit) {
    constexpr uint32_t kQueryCount = 1;

    wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    // Set all bits in buffer to check 0 is correctly written if there is no sample passed the
    // occlusion testing
    queue.WriteBuffer(destination, 0, &kSentinelValue, sizeof(kSentinelValue));

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    renderPass.renderPassInfo.occlusionQuerySet = querySet;

    // Do an occlusion query with a draw call
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.BeginOcclusionQuery(0);
    pass.Draw(3);
    pass.EndOcclusionQuery();
    pass.End();
    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Do another occlusion query with no draw calls, rewriting the same index.
    encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder rewritePass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    rewritePass.BeginOcclusionQuery(0);
    rewritePass.EndOcclusionQuery();
    rewritePass.End();
    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER(destination, 0, sizeof(uint64_t),
                  new OcclusionExpectation(OcclusionExpectation::Result::Zero));
}

// Test that resetting an occlusion query to zero works when a draw is done where all primitives
// fail the depth test.
TEST_P(OcclusionQueryTests, RewriteToZeroWithDraw) {
    constexpr uint32_t kQueryCount = 1;

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;

    // Enable depth and stencil tests and set comparison tests to never pass.
    wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil(kDepthStencilFormat);
    depthStencil->depthCompare = wgpu::CompareFunction::Never;
    depthStencil->stencilFront.compare = wgpu::CompareFunction::Never;
    depthStencil->stencilBack.compare = wgpu::CompareFunction::Never;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&descriptor);

    wgpu::Texture renderTarget = CreateRenderTexture(kColorFormat);
    wgpu::TextureView renderTargetView = renderTarget.CreateView();

    wgpu::Texture depthTexture = CreateRenderTexture(kDepthStencilFormat);
    wgpu::TextureView depthTextureView = depthTexture.CreateView();

    wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    // Set all bits in buffer to check 0 is correctly written if there is no sample passed the
    // occlusion testing
    queue.WriteBuffer(destination, 0, &kSentinelValue, sizeof(kSentinelValue));

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        renderPass.renderPassInfo.occlusionQuerySet = querySet;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.BeginOcclusionQuery(0);
        pass.Draw(3);
        pass.EndOcclusionQuery();
        pass.End();
    }
    {
        utils::ComboRenderPassDescriptor renderPass({renderTargetView}, depthTextureView);
        renderPass.occlusionQuerySet = querySet;

        wgpu::RenderPassEncoder rewritePass = encoder.BeginRenderPass(&renderPass);
        rewritePass.SetPipeline(renderPipeline);
        rewritePass.BeginOcclusionQuery(0);
        rewritePass.Draw(3);
        rewritePass.EndOcclusionQuery();
        rewritePass.End();
    }
    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER(destination, 0, sizeof(uint64_t),
                  new OcclusionExpectation(OcclusionExpectation::Result::Zero));
}

// Test resolving occlusion query to the destination buffer with offset
TEST_P(OcclusionQueryTests, ResolveToBufferWithOffset) {
    constexpr uint32_t kQueryCount = 2;

    wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    renderPass.renderPassInfo.occlusionQuerySet = querySet;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.BeginOcclusionQuery(0);
    pass.Draw(3);
    pass.EndOcclusionQuery();
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    constexpr uint64_t kBufferSize = kQueryCount * sizeof(uint64_t) + kMinDestinationOffset;
    constexpr uint64_t kCount = kQueryCount + kMinCount;

    // Resolve the query result to first slot in the buffer, other slots should not be written.
    {
        wgpu::Buffer destination = CreateResolveBuffer(kBufferSize);
        // Set sentinel values to check the query is resolved to the correct slot of the buffer.
        std::vector<uint64_t> sentinelValues(kCount, kSentinelValue);
        queue.WriteBuffer(destination, 0, sentinelValues.data(), kBufferSize);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, 1, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, sizeof(uint64_t),
                      new OcclusionExpectation(OcclusionExpectation::Result::NonZero));
        EXPECT_BUFFER_U64_RANGE_EQ(sentinelValues.data(), destination, sizeof(uint64_t),
                                   kCount - 1);
    }

    // Resolve the query result to second slot in the buffer, the first one should not be written.
    {
        wgpu::Buffer destination = CreateResolveBuffer(kBufferSize);
        // Set sentinel values to check the query is resolved to the correct slot of the buffer.
        std::vector<uint64_t> sentinelValues(kCount, kSentinelValue);
        queue.WriteBuffer(destination, 0, sentinelValues.data(), kBufferSize);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, 1, destination, kMinDestinationOffset);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U64_RANGE_EQ(sentinelValues.data(), destination, 0, kMinCount);
        EXPECT_BUFFER(destination, kMinDestinationOffset, sizeof(uint64_t),
                      new OcclusionExpectation(OcclusionExpectation::Result::NonZero));
    }
}

class PipelineStatisticsQueryTests : public QueryTests {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Skip all tests if pipeline statistics feature is not supported
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::PipelineStatisticsQuery}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::PipelineStatisticsQuery})) {
            requiredFeatures.push_back(wgpu::FeatureName::PipelineStatisticsQuery);
        }

        return requiredFeatures;
    }

    wgpu::QuerySet CreateQuerySetForPipelineStatistics(
        uint32_t queryCount,
        std::vector<wgpu::PipelineStatisticName> pipelineStatistics = {}) {
        wgpu::QuerySetDescriptor descriptor;
        descriptor.count = queryCount;
        descriptor.type = wgpu::QueryType::PipelineStatistics;

        if (pipelineStatistics.size() > 0) {
            descriptor.pipelineStatistics = pipelineStatistics.data();
            descriptor.pipelineStatisticsCount = pipelineStatistics.size();
        }
        return device.CreateQuerySet(&descriptor);
    }
};

// Test creating query set with the type of PipelineStatistics
TEST_P(PipelineStatisticsQueryTests, QuerySetCreation) {
    // Zero-sized query set is allowed.
    CreateQuerySetForPipelineStatistics(0, {wgpu::PipelineStatisticName::ClipperInvocations,
                                            wgpu::PipelineStatisticName::VertexShaderInvocations});

    CreateQuerySetForPipelineStatistics(1, {wgpu::PipelineStatisticName::ClipperInvocations,
                                            wgpu::PipelineStatisticName::VertexShaderInvocations});
}

class TimestampExpectation : public detail::Expectation {
  public:
    ~TimestampExpectation() override = default;

    // Expect the timestamp results are greater than 0.
    testing::AssertionResult Check(const void* data, size_t size) override {
        ASSERT(size % sizeof(uint64_t) == 0);
        const uint64_t* timestamps = static_cast<const uint64_t*>(data);
        for (size_t i = 0; i < size / sizeof(uint64_t); i++) {
            if (timestamps[i] == 0) {
                return testing::AssertionFailure()
                       << "Expected data[" << i << "] to be greater than 0." << std::endl;
            }
        }

        return testing::AssertionSuccess();
    }
};

class TimestampQueryTests : public QueryTests {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Skip all tests if timestamp feature is not supported
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::TimestampQuery}));

        // Create basic compute pipeline
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @compute @workgroup_size(1)
            fn main() {
            })");

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = module;
        csDesc.compute.entryPoint = "main";
        computePipeline = device.CreateComputePipeline(&csDesc);
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::TimestampQuery})) {
            requiredFeatures.push_back(wgpu::FeatureName::TimestampQuery);
        }
        return requiredFeatures;
    }

    wgpu::QuerySet CreateQuerySetForTimestamp(uint32_t queryCount) {
        wgpu::QuerySetDescriptor descriptor;
        descriptor.count = queryCount;
        descriptor.type = wgpu::QueryType::Timestamp;
        return device.CreateQuerySet(&descriptor);
    }

    wgpu::RenderPipeline CreateRenderPipeline(bool hasFragmentStage = true) {
        utils::ComboRenderPipelineDescriptor descriptor;

        descriptor.vertex.module = utils::CreateShaderModule(device, R"(
                @vertex
                fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                    var pos = array(
                        vec2f( 1.0,  1.0),
                        vec2f(-1.0, -1.0),
                        vec2f( 1.0, -1.0));
                    return vec4f(pos[VertexIndex], 0.0, 1.0);
                })");

        if (hasFragmentStage) {
            descriptor.cFragment.module = utils::CreateShaderModule(device, R"(
                @fragment fn main() -> @location(0) vec4f {
                    return vec4f(0.0, 1.0, 0.0, 1.0);
                })");
        } else {
            descriptor.fragment = nullptr;
            descriptor.EnableDepthStencil(kDepthStencilFormat);
        }

        return device.CreateRenderPipeline(&descriptor);
    }

    void EncodeComputeTimestampWrites(
        const wgpu::CommandEncoder& encoder,
        const std::vector<wgpu::ComputePassTimestampWrite>& timestampWrites,
        bool hasPipeline = true) {
        wgpu::ComputePassDescriptor descriptor;
        descriptor.timestampWriteCount = timestampWrites.size();
        descriptor.timestampWrites = timestampWrites.data();

        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&descriptor);
        if (hasPipeline) {
            pass.SetPipeline(computePipeline);
            pass.DispatchWorkgroups(1, 1, 1);
        }
        pass.End();
    }

    void EncodeRenderTimestampWrites(
        const wgpu::CommandEncoder& encoder,
        const std::vector<wgpu::RenderPassTimestampWrite>& timestampWrites,
        bool hasPipeline = true,
        bool hasFragmentStage = true) {
        wgpu::Texture depthTexture = CreateRenderTexture(kDepthStencilFormat);
        utils::ComboRenderPassDescriptor renderPassDesc =
            hasFragmentStage
                ? utils::ComboRenderPassDescriptor({CreateRenderTexture(kColorFormat).CreateView()})
                : utils::ComboRenderPassDescriptor(
                      {}, CreateRenderTexture(kDepthStencilFormat).CreateView());
        renderPassDesc.timestampWriteCount = timestampWrites.size();
        renderPassDesc.timestampWrites = timestampWrites.data();

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
        if (hasPipeline) {
            wgpu::RenderPipeline renderPipeline = CreateRenderPipeline(hasFragmentStage);
            pass.SetPipeline(renderPipeline);
            pass.Draw(3);
        }
        pass.End();
    }

    void TestTimestampWritesOnComputePass(
        const std::vector<wgpu::ComputePassTimestampWrite>& timestampWrites,
        const std::vector<wgpu::ComputePassTimestampWrite>& timestampWritesOnAnotherPass = {},
        bool hasPipeline = true) {
        size_t queryCount = timestampWrites.size() + timestampWritesOnAnotherPass.size();
        // The destination buffer offset must be a multiple of 256.
        wgpu::Buffer destination =
            CreateResolveBuffer(queryCount * kMinDestinationOffset + sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeComputeTimestampWrites(encoder, timestampWrites, hasPipeline);

        // Begin another compute pass if the timestampWritesOnAnotherPass is set.
        if (!timestampWritesOnAnotherPass.empty()) {
            EncodeComputeTimestampWrites(encoder, timestampWritesOnAnotherPass, hasPipeline);
        }

        // Resolve queries one by one because the query set at the beginning of pass may be
        // different with the one at the end of pass.
        for (size_t i = 0; i < timestampWrites.size(); i++) {
            encoder.ResolveQuerySet(timestampWrites[i].querySet, timestampWrites[i].queryIndex, 1,
                                    destination, i * kMinDestinationOffset);
        }
        for (size_t i = 0; i < timestampWritesOnAnotherPass.size(); i++) {
            encoder.ResolveQuerySet(timestampWritesOnAnotherPass[i].querySet,
                                    timestampWritesOnAnotherPass[i].queryIndex, 1, destination,
                                    (timestampWrites.size() + i) * kMinDestinationOffset);
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        for (size_t i = 0; i < queryCount; i++) {
            EXPECT_BUFFER(destination, i * kMinDestinationOffset, sizeof(uint64_t),
                          new TimestampExpectation);
        }
    }

    void TestTimestampWritesOnRenderPass(
        const std::vector<wgpu::RenderPassTimestampWrite>& timestampWrites,
        const std::vector<wgpu::RenderPassTimestampWrite>& timestampWritesOnAnotherPass = {},
        bool hasPipeline = true,
        bool hasFragmentStage = true) {
        size_t queryCount = timestampWrites.size() + timestampWritesOnAnotherPass.size();
        // The destination buffer offset must be a multiple of 256.
        wgpu::Buffer destination =
            CreateResolveBuffer(queryCount * kMinDestinationOffset + sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeRenderTimestampWrites(encoder, timestampWrites, hasPipeline, hasFragmentStage);

        // Begin another render pass if the timestampWritesOnAnotherPass is set.
        if (!timestampWritesOnAnotherPass.empty()) {
            EncodeRenderTimestampWrites(encoder, timestampWritesOnAnotherPass, hasPipeline,
                                        hasFragmentStage);
        }

        // Resolve queries one by one because the query set at the beginning of pass may be
        // different with the one at the end of pass.
        for (size_t i = 0; i < timestampWrites.size(); i++) {
            encoder.ResolveQuerySet(timestampWrites[i].querySet, timestampWrites[i].queryIndex, 1,
                                    destination, i * kMinDestinationOffset);
        }

        for (size_t i = 0; i < timestampWritesOnAnotherPass.size(); i++) {
            encoder.ResolveQuerySet(timestampWritesOnAnotherPass[i].querySet,
                                    timestampWritesOnAnotherPass[i].queryIndex, 1, destination,
                                    (timestampWrites.size() + i) * kMinDestinationOffset);
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        for (size_t i = 0; i < queryCount; i++) {
            EXPECT_BUFFER(destination, i * kMinDestinationOffset, sizeof(uint64_t),
                          new TimestampExpectation);
        }
    }

  private:
    wgpu::ComputePipeline computePipeline;
};

// Test creating query set with the type of Timestamp
TEST_P(TimestampQueryTests, QuerySetCreation) {
    // Zero-sized query set is allowed.
    CreateQuerySetForTimestamp(0);

    CreateQuerySetForTimestamp(1);
}

// Test calling timestamp query from command encoder
TEST_P(TimestampQueryTests, TimestampOnCommandEncoder) {
    constexpr uint32_t kQueryCount = 2;

    // Write timestamp with different query indexes
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.WriteTimestamp(querySet, 1);
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    }

    // Write timestamp with same query index outside pass on same encoder
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.WriteTimestamp(querySet, 1);
        encoder.WriteTimestamp(querySet, 0);
        encoder.WriteTimestamp(querySet, 1);
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    }
}

// Test timestampWrites with query set in compute pass descriptor
TEST_P(TimestampQueryTests, TimestampWritesQuerySetOnComputePass) {
    // TODO (dawn:1473): Metal bug which fails to store GPU counters to different sample buffer.
    DAWN_SUPPRESS_TEST_IF(IsMacOS() && IsMetal() && IsApple());

    // Set timestampWrites with different query set on same compute pass
    wgpu::QuerySet querySet0 = CreateQuerySetForTimestamp(1);
    wgpu::QuerySet querySet1 = CreateQuerySetForTimestamp(1);

    TestTimestampWritesOnComputePass({{querySet0, 0, wgpu::ComputePassTimestampLocation::Beginning},
                                      {querySet1, 0, wgpu::ComputePassTimestampLocation::End}});
}

// Test timestampWrites with query index in compute pass descriptor
TEST_P(TimestampQueryTests, TimestampWritesQueryIndexOnComputePass) {
    // Set timestampWrites with different query indexes and locations, not need test write same
    // query index due to it's not allowed on compute pass.
    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(2);

    TestTimestampWritesOnComputePass({{querySet, 0, wgpu::ComputePassTimestampLocation::Beginning},
                                      {querySet, 1, wgpu::ComputePassTimestampLocation::End}});
}

// Test timestampWrites with timestamp location in compute pass descriptor
TEST_P(TimestampQueryTests, TimestampWritesLocationOnComputePass) {
    constexpr uint32_t kQueryCount = 2;

    // Set timestampWrites with only one value of ComputePassTimestampLocation
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);

        TestTimestampWritesOnComputePass(
            {{querySet, 0, wgpu::ComputePassTimestampLocation::Beginning}});

        TestTimestampWritesOnComputePass({{querySet, 1, wgpu::ComputePassTimestampLocation::End}});
    }

    // Set timestampWrites with same location on different compute pass
    {
        wgpu::QuerySet querySet0 = CreateQuerySetForTimestamp(1);
        wgpu::QuerySet querySet1 = CreateQuerySetForTimestamp(1);

        TestTimestampWritesOnComputePass(
            {{querySet0, 0, wgpu::ComputePassTimestampLocation::Beginning}},
            {{querySet1, 0, wgpu::ComputePassTimestampLocation::Beginning}});
    }
}

// Test timestampWrites on compute pass without pipeline
TEST_P(TimestampQueryTests, TimestampWritesOnComputePassWithNoPipline) {
    // TODO (dawn:1473): Metal fails to store GPU counters to sampleBufferAttachments on empty
    // encoders.
    DAWN_SUPPRESS_TEST_IF(IsMacOS() && IsMetal() && IsApple());

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(2);

    TestTimestampWritesOnComputePass({{querySet, 0, wgpu::ComputePassTimestampLocation::Beginning},
                                      {querySet, 1, wgpu::ComputePassTimestampLocation::End}},
                                     {}, false);
}

// Test timestampWrites with query set in render pass descriptor
TEST_P(TimestampQueryTests, TimestampWritesQuerySetOnRenderPass) {
    // TODO (dawn:1473): Metal bug which fails to store GPU counters to different sample buffer.
    DAWN_SUPPRESS_TEST_IF(IsMacOS() && IsMetal() && IsApple());

    // Set timestampWrites with different query set on same render pass
    wgpu::QuerySet querySet0 = CreateQuerySetForTimestamp(1);
    wgpu::QuerySet querySet1 = CreateQuerySetForTimestamp(1);

    TestTimestampWritesOnRenderPass({{querySet0, 0, wgpu::RenderPassTimestampLocation::Beginning},
                                     {querySet1, 0, wgpu::RenderPassTimestampLocation::End}});
}

// Test timestampWrites with query index in compute pass descriptor
TEST_P(TimestampQueryTests, TimestampWritesQueryIndexOnRenderPass) {
    // Set timestampWrites with different query indexes and locations, not need test write same
    // query index due to it's not allowed on render pass.
    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(2);

    TestTimestampWritesOnRenderPass({{querySet, 0, wgpu::RenderPassTimestampLocation::Beginning},
                                     {querySet, 1, wgpu::RenderPassTimestampLocation::End}});
}

// Test timestampWrites with timestamp location in render pass descriptor
TEST_P(TimestampQueryTests, TimestampWritesLocationOnRenderPass) {
    // Set timestampWrites with only one value of RenderPassTimestampLocation
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(2);

        TestTimestampWritesOnRenderPass(
            {{querySet, 0, wgpu::RenderPassTimestampLocation::Beginning}});

        TestTimestampWritesOnRenderPass({{querySet, 1, wgpu::RenderPassTimestampLocation::End}});
    }

    // Set timestampWrites with same location on different render pass
    {
        wgpu::QuerySet querySet0 = CreateQuerySetForTimestamp(1);
        wgpu::QuerySet querySet1 = CreateQuerySetForTimestamp(1);

        TestTimestampWritesOnRenderPass(
            {{querySet0, 0, wgpu::RenderPassTimestampLocation::Beginning}},
            {{querySet1, 0, wgpu::RenderPassTimestampLocation::Beginning}});
    }
}

// Test timestampWrites on render pass without pipeline
TEST_P(TimestampQueryTests, TimestampWritesOnRenderPassWithNoPipline) {
    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(2);
    TestTimestampWritesOnRenderPass({{querySet, 0, wgpu::RenderPassTimestampLocation::Beginning},
                                     {querySet, 1, wgpu::RenderPassTimestampLocation::End}},
                                    {}, false);
}

// Test timestampWrites on render pass with pipeline but no fragment stage
TEST_P(TimestampQueryTests, TimestampWritesOnRenderPassWithOnlyVertexStage) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("use_placeholder_fragment_in_vertex_only_pipeline"));

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(2);
    TestTimestampWritesOnRenderPass({{querySet, 0, wgpu::RenderPassTimestampLocation::Beginning},
                                     {querySet, 1, wgpu::RenderPassTimestampLocation::End}},
                                    {}, true, false);
}

// Test resolving timestamp query from another different encoder
TEST_P(TimestampQueryTests, ResolveFromAnotherEncoder) {
    constexpr uint32_t kQueryCount = 2;

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

    wgpu::CommandEncoder timestampEncoder = device.CreateCommandEncoder();
    timestampEncoder.WriteTimestamp(querySet, 0);
    timestampEncoder.WriteTimestamp(querySet, 1);
    wgpu::CommandBuffer timestampCommands = timestampEncoder.Finish();
    queue.Submit(1, &timestampCommands);

    wgpu::CommandEncoder resolveEncoder = device.CreateCommandEncoder();
    resolveEncoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer resolveCommands = resolveEncoder.Finish();
    queue.Submit(1, &resolveCommands);

    EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
}

// Test resolving timestamp query correctly if the queries are written sparsely
TEST_P(TimestampQueryTests, ResolveSparseQueries) {
    constexpr uint32_t kQueryCount = 4;

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    // Set sentinel values to check the queries are resolved correctly if the queries are
    // written sparsely
    std::vector<uint64_t> sentinelValues{0, kSentinelValue, 0, kSentinelValue};
    queue.WriteBuffer(destination, 0, sentinelValues.data(), kQueryCount * sizeof(uint64_t));

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.WriteTimestamp(querySet, 0);
    encoder.WriteTimestamp(querySet, 2);
    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER(destination, 0, sizeof(uint64_t), new TimestampExpectation);
    // The query with no value written should be resolved to 0.
    EXPECT_BUFFER_U64_RANGE_EQ(&kZero, destination, sizeof(uint64_t), 1);
    EXPECT_BUFFER(destination, 2 * sizeof(uint64_t), sizeof(uint64_t), new TimestampExpectation);
    // The query with no value written should be resolved to 0.
    EXPECT_BUFFER_U64_RANGE_EQ(&kZero, destination, 3 * sizeof(uint64_t), 1);
}

// Test resolving timestamp query to 0 if all queries are not written
TEST_P(TimestampQueryTests, ResolveWithoutWritten) {
    constexpr uint32_t kQueryCount = 2;

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    // Set sentinel values to check 0 is correctly written if resolving query set with no
    // query is written
    std::vector<uint64_t> sentinelValues(kQueryCount, kSentinelValue);
    queue.WriteBuffer(destination, 0, sentinelValues.data(), kQueryCount * sizeof(uint64_t));

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint64_t> expectedZeros(kQueryCount);
    EXPECT_BUFFER_U64_RANGE_EQ(expectedZeros.data(), destination, 0, kQueryCount);
}

// Test resolving timestamp query to one slot in the buffer
TEST_P(TimestampQueryTests, ResolveToBufferWithOffset) {
    constexpr uint32_t kQueryCount = 2;
    constexpr uint64_t kBufferSize = kQueryCount * sizeof(uint64_t) + kMinDestinationOffset;
    constexpr uint64_t kCount = kQueryCount + kMinCount;

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);

    // Resolve the query result to first slot in the buffer, other slots should not be written
    {
        wgpu::Buffer destination = CreateResolveBuffer(kBufferSize);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.ResolveQuerySet(querySet, 0, 1, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        std::vector<uint64_t> zeros(kCount - 1, kZero);
        EXPECT_BUFFER(destination, 0, sizeof(uint64_t), new TimestampExpectation);
        EXPECT_BUFFER_U64_RANGE_EQ(zeros.data(), destination, sizeof(uint64_t), kCount - 1);
    }

    // Resolve the query result to the buffer with offset, the slots before the offset
    // should not be written
    {
        wgpu::Buffer destination = CreateResolveBuffer(kBufferSize);
        // Set sentinel values to check the query is resolved to the correct slot of the buffer.
        std::vector<uint64_t> sentinelValues(kCount, kZero);
        queue.WriteBuffer(destination, 0, sentinelValues.data(), kBufferSize);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.ResolveQuerySet(querySet, 0, 1, destination, kMinDestinationOffset);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        std::vector<uint64_t> zeros(kMinCount, kZero);
        EXPECT_BUFFER_U64_RANGE_EQ(zeros.data(), destination, 0, kMinCount);
        EXPECT_BUFFER(destination, kMinDestinationOffset, sizeof(uint64_t),
                      new TimestampExpectation);
    }
}

// Test resolving a query set twice into the same destination buffer with potentially overlapping
// ranges
TEST_P(TimestampQueryTests, ResolveTwiceToSameBuffer) {
    constexpr uint32_t kQueryCount = kMinCount + 2;

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    for (uint32_t i = 0; i < kQueryCount; i++) {
        encoder.WriteTimestamp(querySet, i);
    }
    encoder.ResolveQuerySet(querySet, 0, kMinCount + 1, destination, 0);
    encoder.ResolveQuerySet(querySet, kMinCount, 2, destination, kMinDestinationOffset);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
}

// Test calling WriteTimestamp many times into separate query sets.
// Regression test for crbug.com/dawn/1603.
TEST_P(TimestampQueryTests, ManyWriteTimestampDistinctQuerySets) {
    // TODO(crbug.com/dawn/1829): Avoid OOM on Apple GPUs.
    DAWN_SUPPRESS_TEST_IF(IsApple());

    constexpr uint32_t kQueryCount = 100;
    // Write timestamp with a different query sets many times
    for (uint32_t i = 0; i < kQueryCount; ++i) {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(1);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Destroy the query set so we don't OOM.
        querySet.Destroy();
        // Make sure the queue is idle so the query set is definitely destroyed.
        WaitForAllOperations();
    }
}

class TimestampQueryInsidePassesTests : public TimestampQueryTests {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Skip all tests if timestamp feature is not supported
        DAWN_TEST_UNSUPPORTED_IF(
            !SupportsFeatures({wgpu::FeatureName::TimestampQueryInsidePasses}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::TimestampQueryInsidePasses})) {
            requiredFeatures.push_back(wgpu::FeatureName::TimestampQueryInsidePasses);
            // The timestamp query feature must be supported if the timestamp query inside passes
            // feature is supported. Enable timestamp query for testing queries overwrite inside and
            // outside of the passes.
            requiredFeatures.push_back(wgpu::FeatureName::TimestampQuery);
        }
        return requiredFeatures;
    }
};

// Test calling timestamp query from render pass encoder
TEST_P(TimestampQueryInsidePassesTests, FromOnRenderPass) {
    constexpr uint32_t kQueryCount = 2;

    // Write timestamp with different query indexes
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.WriteTimestamp(querySet, 0);
        pass.WriteTimestamp(querySet, 1);
        pass.End();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    }

    // Write timestamp with same query index, not need test rewrite inside render pass due to it's
    // not allowed
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.WriteTimestamp(querySet, 1);

        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.WriteTimestamp(querySet, 0);
        pass.WriteTimestamp(querySet, 1);
        pass.End();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    }
}

// Test calling timestamp query from compute pass encoder
TEST_P(TimestampQueryInsidePassesTests, FromComputePass) {
    constexpr uint32_t kQueryCount = 2;

    // Write timestamp with different query indexes
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.WriteTimestamp(querySet, 0);
        pass.WriteTimestamp(querySet, 1);
        pass.End();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    }

    // Write timestamp with same query index on both the outside and the inside of the compute pass
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.WriteTimestamp(querySet, 1);

        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.WriteTimestamp(querySet, 0);
        pass.WriteTimestamp(querySet, 1);
        pass.End();

        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    }

    // Write timestamp with same query index inside compute pass
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.WriteTimestamp(querySet, 0);
        pass.WriteTimestamp(querySet, 1);
        pass.WriteTimestamp(querySet, 0);
        pass.WriteTimestamp(querySet, 1);
        pass.End();

        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    }
}

DAWN_INSTANTIATE_TEST(OcclusionQueryTests,
                      D3D12Backend(),
                      MetalBackend(),
                      MetalBackend({"metal_fill_empty_occlusion_queries_with_zero"}),
                      VulkanBackend());
DAWN_INSTANTIATE_TEST(PipelineStatisticsQueryTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
DAWN_INSTANTIATE_TEST(TimestampQueryTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
DAWN_INSTANTIATE_TEST(TimestampQueryInsidePassesTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
