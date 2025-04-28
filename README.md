## Popnet Chiplet 2.0

### Overview  
This is a new version of Popnet Chiplet. The underlying principles and theoretical foundations remain unchanged. The updates mainly involve code optimizations, such as replacing several data structures, refactoring parts of the code using modern C++, and adding necessary comments for improved readability.

### New Features  
This version introduces new functionality, including support for reading a `.json` file to configure simulator parameters. For details, please refer to the “config.json example” section in `./config.json`.

### Build  
You can build the executable by running:  
```
make build
```  
This will produce an executable file at `./build/popnet`.

### Test  
You can also run:  
```
make test
```  
to perform a simple test.

### Additional Notes  
This project currently lacks comprehensive testing. Contributions in the form of additional tests or benchmarks are highly welcome. If you are interested in helping with testing or providing benchmarks, please feel free to contact me. For more related documentation and detailed information, you may also refer to the repositories listed under the **Original Repositories** section below.

### Old Version  
The old version’s documentation files and user guides are preserved in the `./docs` directory.

### Original Repositories  
- Popnet Chiplet code:  
  https://github.com/baikeina/popnet_chiplet  
- Popnet code:  
  https://github.com/karellincoln/popnet  

If you have any questions, feel free to contact me.
