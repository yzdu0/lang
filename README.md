# lang

in progress programming language project (fr this time)

- Includes compiler (tokenizer, parser, code generation) to bytecode.
- Includes process virtual machine to execute bytecode

Features include
- Basic types (integer, dynamic array w/ generic elements)
- Structs w/generics
- Function references as variables


```rust
struct Matrix2x2<T : Type> {
  a : T, b : T, c : T, d : T 
};

fn update<T : Type>(index : Int, val : Matrix2x2<T>) -> Int {
    return updateNode(1, 1, BASE, index, val);
}

fn updateNode<T : Type>(
    node : Int, left : Int, right : Int, index : Int,
    val : Matrix2x2<T>
) -> Int {
    if (left == right) {
        tree[node] = normalise(val);
        return 0;
    }
    let mid : Int = left + (right - left) / 2;
    if (index <= mid) {
        updateNode(node * 2, left, mid, index, val);
    } else {
        updateNode(node * 2 + 1, mid + 1, right, index, val);
    }

    multiplyInto(
        tree[node],
        tree[node * 2],
        tree[node * 2 + 1]
    );
    return 0;
}


fn fft(
    real : DynamicArray<Int>,
    imaginary : DynamicArray<Int>,
    inverse : Int
    ) -> Int {
    let size : Int = len(real);

    # Bit-reversal permutation.
    let reversed : Int = 0;
    let index : Int = 1;
    while(index < size){
        let bit : Int = size / 2;
        while(reversed >= bit){
            reversed = reversed - bit;
            bit = bit / 2;
        }
        reversed = reversed + bit;
        if(index < reversed){
            swapComplex(real, imaginary, index, reversed);
        }
        index = index + 1;
    }
...
```
