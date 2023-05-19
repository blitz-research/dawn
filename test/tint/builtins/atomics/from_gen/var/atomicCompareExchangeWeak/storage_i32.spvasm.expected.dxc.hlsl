struct x__atomic_compare_exchange_resulti32 {
  int old_value;
  bool exchanged;
};

RWByteAddressBuffer sb_rw : register(u0);

struct atomic_compare_exchange_weak_ret_type {
  int old_value;
  bool exchanged;
};

atomic_compare_exchange_weak_ret_type sb_rwatomicCompareExchangeWeak(uint offset, int compare, int value) {
  atomic_compare_exchange_weak_ret_type result=(atomic_compare_exchange_weak_ret_type)0;
  sb_rw.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


void atomicCompareExchangeWeak_1bd40a() {
  int arg_1 = 0;
  int arg_2 = 0;
  x__atomic_compare_exchange_resulti32 res = (x__atomic_compare_exchange_resulti32)0;
  arg_1 = 1;
  arg_2 = 1;
  const int x_23 = arg_2;
  const int x_24 = arg_1;
  const atomic_compare_exchange_weak_ret_type tint_symbol = sb_rwatomicCompareExchangeWeak(0u, x_24, x_23);
  const int old_value_1 = tint_symbol.old_value;
  const int x_25 = old_value_1;
  const x__atomic_compare_exchange_resulti32 tint_symbol_1 = {x_25, (x_25 == x_23)};
  res = tint_symbol_1;
  return;
}

void fragment_main_1() {
  atomicCompareExchangeWeak_1bd40a();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicCompareExchangeWeak_1bd40a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
