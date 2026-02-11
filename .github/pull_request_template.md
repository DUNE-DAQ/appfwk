# Description

_If full description and testing details are included on a parent issue, please link to that here._
See issue # for details

_Otherwise, please include a summary of the change and which issue is fixed (if any).
Include relevant motivation and context, including a target environment and dunedaq version if known.
Also list any dependencies that are required for this change._
Addresses issue # 

_Please also include instructions for how a reviewer can test your changes._


## Type of change

- [ ] Documentation (non-breaking change that adds or improves the documentation)
- [ ] New feature or enhancement (non-breaking change which adds functionality)
- [ ] Optimization (non-breaking change that improves code/performance)
- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] Breaking change (whatever its nature)

## Testing checklist

- [ ] Unit tests pass (e.g. `dbt-build --unittest`)
- [ ] Minimal system quicktest passes (`pytest -s minimal_system_quick_test.py`)
- [ ] Full set of integration tests pass (`dunedaq_integtest_bundle.sh`)
- [ ] Python tests pass if applicable (e.g. `python -m pytest`)
- [ ] Pre-commit hooks run successfully if applicable (e.g. `pre-commit run --all-files`)

_Comments here on the testing_

## Further checks

- [ ] Code is commented where needed, particularly in hard-to-understand areas
- [ ] Code style is correct (`dbt-build --lint`, and/or see https://dune-daq-sw.readthedocs.io/en/latest/packages/styleguide/)
- [ ] If applicable, new tests have been added or an issue has been opened to tackle that in the future.
  (Indicate issue here: # (issue))

