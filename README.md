# C and x86-64 Dot-Product Kernel Comparison

## 1. Project Overview

This project calculates the dot product of two double-precision vectors using two equivalent kernels:

1. A C kernel
2. An x86-64 assembly kernel

For vectors \(A\) and \(B\) containing \(n\) elements, the calculation is:

\[
\text{sdot} = \sum_{i=0}^{n-1} A[i] \times B[i]
\]

In words, `sdot` is the sum of `A[i]` multiplied by `B[i]`, from `i = 0` to `n - 1`.

Both kernels perform the same calculation. The assembly implementation uses scalar double-precision XMM instructions. Each kernel is executed 20 times, and the program reports the average kernel execution time. Vector allocation and initialization occur outside the timed section. After timing, the program compares the two outputs to verify correctness.

## 2. Project Structure

The repository contains the following source, configuration, documentation, and media paths. Generated object files, executables, and debugging files may also be present locally after a build, but they are excluded here because they are build outputs covered by `.gitignore`.

```text
DotProductProject/
├── main.c
├── dot_product.c
├── dot_product.asm
├── DotProductProject.sln
├── DotProductProject.vcxproj
├── DotProductProject.vcxproj.filters
├── correctness_check.png
├── README.md
├── .gitignore
├── .vscode/
│   └── tasks.json
```

The correctness screenshot is stored directly in the repository so that GitHub can render it inside this README. Demonstration videos are hosted externally and linked below.

## 3. C Kernel

[`dot_product.c`](dot_product.c) uses a double-precision accumulator and processes one pair of vector elements per iteration. Each iteration multiplies `A[i]` by `B[i]` and adds the product to the running total. After the loop, the function stores the final result through the `sdot` pointer.

The central calculation is:

```c
double sum = 0.0;

for (size_t i = 0; i < n; i++)
{
    sum += vector_a[i] * vector_b[i];
}

*sdot = sum;
```

## 4. x86-64 Assembly Kernel

[`dot_product.asm`](dot_product.asm) uses Microsoft MASM x86-64 syntax and follows the Windows x64 calling convention. Its arguments arrive in these registers:

| Register | Argument |
|---|---|
| `RCX` | Address of vector A |
| `RDX` | Address of vector B |
| `R8` | Number of elements |
| `R9` | Address where the result is stored |

The kernel uses the XMM registers as follows:

| Register | Purpose |
|---|---|
| `XMM0` | Accumulated sum |
| `XMM1` | Current vector product |

The scalar double-precision instructions have these roles:

- `MOVSD` loads one double-precision value.
- `MULSD` multiplies one double-precision value.
- `ADDSD` adds one double-precision value to the accumulator.
- `XORPD` initializes the accumulator to zero.

The important assembly loop is:

```asm
dot_product_loop:
    movsd xmm1, QWORD PTR [rcx]
    mulsd xmm1, QWORD PTR [rdx]
    addsd xmm0, xmm1
    add rcx, 8
    add rdx, 8
    dec r8
    jnz dot_product_loop
```

## 5. Requirements

- Windows 10 or Windows 11
- Visual Studio with the **Desktop development with C++** workload
- MSVC `cl.exe`
- MASM `ml64.exe`
- Git for Windows

Visual Studio Code and the Microsoft C/C++ extension are optional. The included `.vscode/tasks.json` provides the same Debug and Release build commands for users who prefer VS Code.

## 6. Building the Program

### Visual Studio

1. Open `DotProductProject.sln` in Visual Studio.
2. Choose either **Debug** or **Release** from the solution configuration list.
3. Choose **x64** as the solution platform.
4. Select **Build > Build Solution** or press <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>B</kbd>.
5. Select **Debug > Start Without Debugging** or press <kbd>Ctrl</kbd> + <kbd>F5</kbd> to run the program.

Visual Studio builds the C kernels with MSVC, assembles `dot_product.asm` with MASM, and links them into one executable. The outputs are:

- `x64\Debug\dotproduct_debug.exe`
- `x64\Release\dotproduct_release.exe`

### Visual Studio Code alternative

Open Visual Studio Code from the **x64 Native Tools Command Prompt** so that the MSVC and MASM tools are available:

```bat
cd /d "C:\Users\matth\Documents\DotProductProject"
code .
```

#### Release build

1. Press <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>B</kbd>.
2. Select **Build Release x64**.
3. Run the program:

```powershell
.\dotproduct_release.exe
```

#### Debug build

1. Press <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd>.
2. Select **Tasks: Run Task**.
3. Select **Build Debug x64**.
4. Run the program:

```powershell
.\dotproduct_debug.exe
```

The Release task enables C compiler optimization, while the Debug task disables C compiler optimization. The assembly algorithm remains essentially the same in both configurations.

## 7. Correctness Verification

Both kernels must produce matching results. Because floating-point values should not normally be compared directly with `==`, the program checks the difference using a relative floating-point tolerance.

The verified output for a vector size of \(2^{24}\) is:

```text
C result:       310378446.0000000000
x86-64 result:  310378446.0000000000
Correctness check: PASSED
```

`PASSED` confirms that the x86-64 assembly kernel produced the expected result.

## 8. Comparative Execution Time

The following measurements were recorded on a 12th Gen Intel Core i5-12500H running 64-bit Windows. Each value is the arithmetic mean of 20 kernel executions. A warm-up call was performed before measurement, and vector allocation and initialization were excluded from the timed region. `QueryPerformanceCounter` measured only the call to the selected dot-product kernel.

The required \(2^{30}\) case would require approximately 16 GiB for the two input vectors alone. Because that size was not practical on the test computer, the specification's permitted reduced size of \(2^{28}\) was used.

### Release build (`/O2`)

| Vector Size | Elements | C Average | x86-64 Average | C / x86-64 Ratio | Faster Kernel |
|---|---:|---:|---:|---:|---|
| \(2^{20}\) | 1,048,576 | 0.969545 ms | 0.998995 ms | 0.971x | C by 2.9% |
| \(2^{24}\) | 16,777,216 | 17.349870 ms | 16.965565 ms | 1.023x | x86-64 by 2.2% |
| \(2^{28}\) | 268,435,456 | 288.886575 ms | 289.906800 ms | 0.996x | C by 0.4% |

### Debug build (`/Od`)

| Vector Size | Elements | C Average | x86-64 Average | C / x86-64 Ratio | Faster Kernel |
|---|---:|---:|---:|---:|---|
| \(2^{20}\) | 1,048,576 | 3.334720 ms | 1.267850 ms | 2.630x | x86-64 by 62.0% |
| \(2^{24}\) | 16,777,216 | 51.431515 ms | 18.070605 ms | 2.846x | x86-64 by 64.9% |
| \(2^{28}\) | 268,435,456 | 840.621760 ms | 297.824735 ms | 2.823x | x86-64 by 64.6% |

The ratio is calculated as `C time / x86-64 time`. A value above 1.0 means the assembly kernel was faster, while a value below 1.0 means the C kernel was faster. Execution times depend on current system load, processor frequency, and memory state, so repeated executions may produce slightly different values.

## 9. Performance Analysis

The results demonstrate why build configuration must be considered when comparing C and assembly. In the Debug build, the C compiler used `/Od`, so it deliberately avoided optimization. The handwritten assembly kernel did not change between configurations and was therefore 2.630x to 2.846x faster than Debug C across the three sizes. This large, consistent advantage comes primarily from comparing deliberately unoptimized C with an already hand-optimized assembly loop.

In the Release build, `/O2` allowed the compiler to optimize the C loop, its address calculations, and loop control. The two Release kernels were within 3% at every tested size. C was 2.9% faster at \(2^{20}\), assembly was 2.2% faster at \(2^{24}\), and C was 0.4% faster at \(2^{28}\). Differences this small can change between benchmark executions as system conditions vary. The appropriate conclusion is that optimized C and the scalar assembly kernel performed comparably—not that either language was universally faster.

The assembly kernel intentionally satisfies the scalar-SIMD requirement by processing one double per iteration with `MOVSD`, `MULSD`, and `ADDSD`. Its single `XMM0` accumulator creates a loop-carried dependency: each `ADDSD` must wait for the previous sum before the next sum can complete. This limits instruction-level parallelism. Multiple independent accumulators could reduce that dependency, but doing so would change the deliberately simple scalar kernel and could alter the order of floating-point addition.

Each element requires two 8-byte loads, or 16 bytes of input traffic, for one multiplication and one addition. Its arithmetic intensity is therefore only 0.125 floating-point operations per byte read. At \(2^{28}\), the two vectors occupy approximately 4 GiB, far beyond the processor's cache capacity. The benchmark consequently depends heavily on main-memory bandwidth and cache behavior, which explains why execution time does not scale solely with the number of arithmetic instructions.

The comparison is fair with respect to the assignment: both kernels receive the same initialized vectors, use the same result type, run 20 times, and are timed only around the kernel call. Warm-up calls reduce first-call effects, and correctness is checked after measurement. Remaining variation can come from operating-system scheduling, background activity, CPU frequency changes, cache state, and thermal conditions. The measurements characterize this test system and should not be interpreted as universal performance values.

## 10–11. C and x86-64 Correctness Screenshot

The following Visual Studio Debug x64 execution shows the C result, x86-64 result, average kernel times, and successful correctness check for all three vector sizes. Each C result exactly matches its corresponding x86-64 result, and every test reports `Correctness check: PASSED`.

![Visual Studio output showing matching C and x86-64 results with successful correctness checks](correctness_check.png)

Timing values in the screenshot come from one Debug execution and can vary between runs because of normal system conditions.

## 12. Demonstration Video

The following demonstration shows the source code, compilation, execution, correctness checks, and performance comparison for the C and x86-64 kernels.

### C and x86-64 Demonstration

[Demonstration video link](https://drive.google.com/file/d/10zYN18WOicmDzyVHx2kNk7Q0ilVZFOFi/view?usp=sharing)

## 13. Memory Requirements

| Vector Size | Approximate Memory for Both Vectors |
|---|---:|
| \(2^{20}\) | 16 MiB |
| \(2^{24}\) | 256 MiB |
| \(2^{28}\) | 4 GiB |
| \(2^{30}\) | 16 GiB |

## 14. Limitations

- Execution times vary between runs.
- The assembly implementation uses scalar SIMD instructions.
- It processes one double per loop iteration.
- Packed instructions such as `MULPD` and `ADDPD` are not used.
- Large vector tests may fail because of insufficient RAM.
- Results depend on the processor, compiler settings, operating system, and current system workload.
- The recorded results should not be treated as universal performance values.

## 15. Conclusion

Both kernels produced identical results for \(2^{20}\), \(2^{24}\), and \(2^{28}\), and every correctness check passed in both Debug and Release builds. Assembly substantially outperformed unoptimized Debug C, while optimized Release C and scalar assembly were within 3% of each other at every tested size. The experiment demonstrates that performance depends on compiler optimization, instruction dependencies, cache capacity, and memory bandwidth—not simply on whether code is written in C or assembly.

The \(2^{30}\) test was not performed because the two input vectors alone require approximately 16 GiB of memory, equal to the system's total installed RAM. Additional memory is required by the operating system and other processes, making that test impractical on the available hardware. The specification explicitly permits reducing this case to \(2^{28}\) or \(2^{29}\), so \(2^{28}\) was measured instead.
