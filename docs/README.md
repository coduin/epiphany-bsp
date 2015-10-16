## Epiphany BSP documentation

### Prerequisites

Generating the documentation pages requires the following programs and packages:

- Sphinx (`python-sphinx` package)
- Breathe (`breathe` python package. First install `python-pip` and run `sudo pip install breathe`)
- Doxygen (`doxygen` package)
- `sphinx_rtd_theme` (comes with `python-sphinx`)

### Building

From the root directory of the Epiphany BSP directory, run

    make -B docs

The resulting files are in `docs/_build/html/`
