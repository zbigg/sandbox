const bar = 10;

function hello(x: number, y: string): string {
    return y + x;
}
/*
function hello(x, y) {
    return y + x;
}
*/

const foo = hello(5, "barz");
console.log(foo)