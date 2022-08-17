fn succ_i32(x: i32) -> i32 {
    x + 1
}

#[test]
fn test_succ() {
    let arg: i32 = 1;
    let expected: i32 = 2;
    assert_eq!(succ_i32(arg), expected);
}

fn succ<T>(x: T) -> T
where
    T: num_traits::PrimInt,
{
    x + num::NumCast::from(1).unwrap()
}

#[test]
fn test_succ_num() {
    let arg_i8: i8 = 1;
    let expected_i8: i8 = 2;
    assert_eq!(succ(arg_i8), expected_i8);

    let arg_u8: u8 = 254;
    let expected_u8: u8 = 255;
    assert_eq!(succ(arg_u8), expected_u8);

    let arg_i64: i64 = 0x700180029003a00f;
    let expected_i64: i64 = 0x700180029003a010;
    assert_eq!(succ(arg_i64), expected_i64);

    let arg_i64: u128 = 0xff00ee00dd00cc00bb00aa009900ffff;
    let expected_i64: u128 = 0xff00ee00dd00cc00bb00aa0099010000;
    assert_eq!(succ(arg_i64), expected_i64);
}

fn main() {}
