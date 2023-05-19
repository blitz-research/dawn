struct x__atomic_compare_exchange_resultu32 {
  uint old_value;
  bool exchanged;
};

RWByteAddressBuffer sb_rw : register(u0);

struct atomic_compare_exchange_weak_ret_type {
  uint old_value;
  bool exchanged;
};

atomic_compare_exchange_weak_ret_type sb_rwatomicCompareExchangeWeak(uint offset, uint compare, uint value) {
  atomic_compare_exchange_weak_ret_type result=(atomic_compare_exchange_weak_ret_type)0;
  sb_rw.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


void atomicCompareExchangeWeak_63d8e6() {
  uint arg_1 = 0u;
  uint arg_2 = 0u;
  x__atomic_compare_exchange_resultu32 res = (x__atomic_compare_exchange_resultu32)0;
  arg_1 = 1u;
  arg_2 = 1u;
  const uint x_21 = arg_2;
  const uint x_22 = arg_1;
  const atomic_compare_exchange_weak_ret_type tint_symbol = sb_rwatomicCompareExchangeWeak(0u, x_22, x_21);
  const uint old_value_1 = tint_symbol.old_value;
  const uint x_23 = old_value_1;
  const x__atomic_compare_exchange_resultu32 tint_symbol_1 = {x_23, (x_23 == x_21)};
  res = tint_symbol_1;
  return;
}

void fragment_main_1() {
  atomicCompareExchangeWeak_63d8e6();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicCompareExchangeWeak_63d8e6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
