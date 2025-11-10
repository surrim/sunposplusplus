# Sunpos++

A C++23 implementation of the solar azimuth formula from Taiping Zhang, Paul W. Stackhouse, Bradley Macpherson and Colleen Mikovitz (2021).

> A solar azimuth formula that renders circumstantial treatment unnecessary without compromising mathematical rigor: Mathematical setup, application and extension of a formula based on the subsolar point and atan2 function
> *Renewable Energy, 172, 1333-1340*

This project provides a compact, self-contained implementation of azimuth computation using the approach described in the paper. It outputs the solar azimuth and zenith for a given date, time, and location.

## Features

- Implements the atan2-based azimuth formula that avoids case-by-case circumstantial adjustments
- Computes subsolar point for a given UTC datetime
- Returns azimuth relative to north
- Dependencies: C++23 (or above) and [p-ranav/argparse](https://github.com/p-ranav/argparse) for CLI argument parsing

## Build & Run

```bash
g++ -std=c++23 *.cpp -o sunpos
./sunpos 51.340333 12.374750 -d "2025-10-31 18:33:00"
# output:
# 2025-10-31 18:33:00	51.340333	12.374750	-26.541346	280.398407
```

Add `-DSP_FLOATING_POINT_TYPE=double` to compiler flags for `double` precision.

## Files

- main.cpp — CLI program
- sunpos.cpp and sunpos.h — contains `compute_sun_position()`, include for own programs
- sunpos.cbp — Code::Blocks project file for quick start

## License

GNU Public License, see LICENSE file.

## References

- Zhang, T., Stackhouse, P.W., Macpherson, 
  B., and Mikovitz, J.C., 2021. A solar azimuth formula that renders 
  circumstantial treatment unnecessary without compromising mathematical 
  rigor: Mathematical setup, application and extension of a formula based 
  on the subsolar point and atan2 function. *Renewable Energy*, 172, 1333-1340. DOI: [https://doi.org/10.1016/j.renene.2021.03.047](https://doi.org/10.1016/j.renene.2021.03.047)

## Contact

For issues or contributions, open an issue or pull request in the repository.
