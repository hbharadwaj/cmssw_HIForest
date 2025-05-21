# HiForest Skimming Documentation

## Overview

The skimming system processes HiForest files using ROOT's RDataFrame to create smaller, analysis-ready files. 

### Key Components

1. **Config File Structure**
- Tree definitions with aliases (e.g., hiEvtAnalyzer/HiTree:hiEvt)
- Branch selections per tree
- I/O paths and processing parameters

2. **Tree Organization**
- Base tree: hiEvtAnalyzer/HiTree (critical for proper branch handling)
- Friend trees with aliases (ggHi, skim, hlt, AK2Z1, etc.)
- Branch name formats vary by tree type

3. **RDataFrame Column Handling**
- Base tree: branches appear without prefix
- Friend trees: branches appear as alias.branch
- Column selection must account for both formats

## Debugging Lessons

1. **Branch Name Resolution**
- Base tree branches have no prefix in RDataFrame
- Friend tree branches follow pattern: alias.branch
- Must check both formats when searching columns

2. **Tree Configuration**
- Base tree must be HiTree for correct event structure
- Friend tree order affects branch naming
- Tree aliases must match config file exactly

3. **Data Processing**
- RDataFrame Snapshot preserves original branch structure
- Friend tree relationships maintained through processing
- Column name mapping handled automatically by ROOT

## TODO List

1. **Performance Optimization**
- [ ] Evaluate RDF column filtering performance
- [ ] Consider implementing batch processing
- [ ] Profile memory usage with large files

2. **Error Handling**
- [ ] Add validation for config file format
- [ ] Improve missing column reporting
- [ ] Add checks for tree/branch existence

3. **Features**
- [ ] Support regex in branch selection
- [ ] Add event filtering options
- [ ] Implement progress reporting

4. **Documentation**
- [ ] Add examples for common use cases
- [ ] Document config file format thoroughly
- [ ] Create troubleshooting guide

## Best Practices

1. **Development**
- Use verbose debug output during development
- Test with small file samples first
- Verify column names in RDataFrame immediately

2. **Configuration**
- Keep base tree (HiTree) first in config
- Use consistent alias naming
- Document branch selection patterns

3. **Validation**
- Check output file size and content
- Verify branch name preservation
- Ensure event counts match expectations

## Common Issues

1. **Missing Branches**
- Check tree/alias names match exactly
- Verify branch exists in input file
- Confirm correct prefix usage

2. **Tree Loading**
- HiTree must be base tree
- Friend trees need proper paths
- Chain addition order matters

3. **Output Format**
- Branch names follow RDataFrame conventions
- Friend tree prefixes preserved
- Original data types maintained

## Future Plans

1. **Automation**
- Automated testing suite
- Config file validation
- Branch existence checking

2. **Extensions**
- Support for more complex event selection
- Dynamic tree structure handling
- Improved progress monitoring

3. **Integration**
- Better CMSSW integration
- Support for different forest formats
- Handling of calibration data
