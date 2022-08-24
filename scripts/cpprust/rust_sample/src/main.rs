use num_bigint::BigInt;

fn succ_i32(x: i32) -> i32 {
    x + 1
}

#[test]
fn test_succ_i32() {
    let arg: i32 = 1;
    let expected: i32 = 2;
    assert_eq!(succ_i32(arg), expected);
}

// fn test_succ_i32_error() {
//    let arg: i16 = 1;
//    let expected: i16 = 2;
//    assert_eq!(succ_i32(arg), expected);
// }

fn succ<T>(x: T) -> T
where
    T: num_traits::PrimInt,
{
    x + num::NumCast::from(1).unwrap()
}

#[test]
fn test_succ() {
    let arg_i8: i8 = 1;
    let expected_i8: i8 = 2;
    assert_eq!(succ(arg_i8), expected_i8);

    let arg_u8: u8 = 254;
    let expected_u8: u8 = 255;
    assert_eq!(succ(arg_u8), expected_u8);

    let arg: i32 = 1;
    let expected: i32 = 2;
    assert_eq!(succ(arg), expected);

    let arg_i64: i64 = 0x700180029003a00f;
    let expected_i64: i64 = 0x700180029003a010;
    assert_eq!(succ(arg_i64), expected_i64);

    let arg_i64: u128 = 0xff00ee00dd00cc00bb00aa009900ffff;
    let expected_i64: u128 = 0xff00ee00dd00cc00bb00aa0099010000;
    assert_eq!(succ(arg_i64), expected_i64);
}

// fn test_succ_error() {
//    let arg: f32 = 1.0;
//    let expected: f32 = 2.0;
//    assert_eq!(succ(arg), expected);
// }

fn succ_big<T>(x: T) -> T
where
    T: std::ops::Add + std::ops::Add<Output = T> + std::convert::From<i8> + std::ops::BitAnd,
{
    x + 1_i8.into()
}

#[test]
fn test_succ_big() {
    let n = BigInt::parse_bytes(b"999999999999999999999999999999999", 10).unwrap();
    let n_plus = BigInt::parse_bytes(b"1000000000000000000000000000000000", 10).unwrap();
    assert_eq!(succ_big(n), n_plus);

    let arg: i64 = 0x2000ffff;
    let expected = 0x20010000;
    assert_eq!(succ_big(arg), expected);
}

// fn test_succ_big_error() {
//    let arg: f32 = 1.0;
//    let expected: f32 = 2.0;
//    assert_eq!(succ_big(arg), expected);
// }

fn main() {}
