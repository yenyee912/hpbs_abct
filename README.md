# hpbs_abct
The repository for Paper: Highly-Probable Boomerang Switch (HPBS) Pattern


# 🔍 ARX Boomerang Trail Search: CHAM-64 & SPARX-64

This repository contains all necessary files to launch a boomerang trail search on two ARX-based block ciphers: **CHAM-64** and **SPARX-64**, using the CryptoSMT framework.

---

## 🚀 Getting Started

### 1. Install Dependencies

- Clone and install [**CryptoSMT**](https://github.com/username/CryptoSMT) along with its required solvers.
- Follow the installation instructions provided in the CryptoSMT repository.

---

### 2. Integrate Files into CryptoSMT

After setting up CryptoSMT, copy the following files and folders into the appropriate directories:

| Source Path                          | Destination Path within CryptoSMT         | Description                                             |
|-------------------------------------|-------------------------------------------|---------------------------------------------------------|
| `abct_cpp/`                         | `CryptoSMT/abct_cpp/`  (root directory)  | C++ code for ABCT probability evaluation                |
| `ciphers/sparxroundBoom.py`         | `CryptoSMT/ciphers/`                      | SMT model for SPARX with HPBS patterns                  |
| `ciphers/chamBoom.py`              | `CryptoSMT/ciphers/`                      | SMT model for CHAM with HPBS patterns                   |
| `cryptanalysis/newBoom.py`         | `CryptoSMT/cryptanalysis/`               | Boomerang search logic                                  |
| `newcryptosmt-arx.py`              | `CryptoSMT/` (root directory)             | Entry point script to launch ARX boomerang search       |
| `examples/chamBoom.yaml`           | `CryptoSMT/examples/`                     | Configuration for CHAM-64 search                        |
| `examples/sparxroundBoom.yaml`     | `CryptoSMT/examples/`                     | Configuration for SPARX-64 search                       |
| `paper/verify_cham_components.cpp` | Standalone C++ program                    | Experimental verification of practical CHAM-64 component differentials |
| `paper/verify_sparx_components.cpp` | Standalone C++ program                   | Experimental verification of practical SPARX-64 component differentials |

---

### 3. Launching the Boomerang Trail Search

Run the following command from the root of your CryptoSMT directory:

```bash
python newcryptosmt-arx.py --inputfile chamBoom.yaml
```

### Experimental Verification

The practical CHAM-64 component trails used in the paper can be sampled with:

```bash
g++ -O3 -std=c++17 paper/verify_cham_components.cpp -o paper/verify_cham_components
./paper/verify_cham_components 4194304 16 0x485042535f414243
```

The first argument is the number of plaintext pairs per key, the second is the number of random keys, and the third is the random seed.

The practical SPARX-64 component trails can be sampled with:

```bash
g++ -O3 -std=c++17 paper/verify_sparx_components.cpp -o paper/verify_sparx_components
./paper/verify_sparx_components 67108864 0x53504152585f4850
```

The lower SPARX component starts at round 1, matching the `X01/Y01 -> X07/Y07` indexing in `examples/sparxroundBoom.yaml`.

The random-sampling check used for Algorithm 2 can be reproduced with:

```bash
g++ -O3 -std=c++17 paper/algo2.cpp -o paper/algo2
./paper/algo2
```

The script first generates complete 16-bit words and then injects the corresponding 4-bit HPBS pattern into the least significant bits. The archived sample output for the `(alpha, alpha') = (0x0, 0x2)` condition is provided in `paper/verification-0x0-0x2.txt`.

## 📌 Notes
chamBoom.py and sparxroundBoom.py are SMT models that integrate Highly Probable Boomerang Switch (HPBS) patterns.

- To apply this technique to a different ARX cipher:

1. Build the cipher's SMT model.

2. Integrate suitable HPBS patterns.

3. Add corresponding code/function to analyse and extract your cipher trail in `cryptanalysis/newBoom.py`.

4. Refer to the related research article for guidance on how to replicate step 1-3.

## 📚 Citation
If you use this tool in your work, please consider citing the associated paper (link/article to be added here).
