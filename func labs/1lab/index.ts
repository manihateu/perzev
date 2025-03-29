function factorial(n: number): number {
    let result = 1;
    for (let i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

function calculateSeriesSum(x: number, eps: number): number {
    let sum = 0; 
    let term = x; 
    let n = 0;   

    while (Math.abs(term) >= eps) {
        sum += term;

        n++;
        const power = 2 * n + 1; 
        const fact = factorial(power); 
        term = Math.pow(x, power) / fact;
    }

    return sum;
}

const x = 1;
const eps = 1e-6;
const result = calculateSeriesSum(x, eps);
console.log(`Сумма ряда при x=${x} и eps=${eps} равна: ${result}`);