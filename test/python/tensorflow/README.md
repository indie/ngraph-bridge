  
## Description
tf_unittest_runner is primarily used to run tensorflow python unit tests using nGraph

## What can be tested

 - Python tests using Tensorflow with nGraph embedded
 - Python tests by patching Tensorflow to allow runnings tests using nGraph.
	```tf_unittest_ngraph.patch```

## Usage

    usage: tf_unittest_runner.py [-h] --tensorflow_path TENSORFLOW_PATH
    
    [--list_tests LIST_TESTS] [--run_test RUN_TEST]
    
    [--run_tests_from_file RUN_TESTS_FROM_FILE]
    
      
    required arguments:
    
    --tensorflow_path TENSORFLOW_PATH
    
    Specify the path where Tensorflow is installed.
    
    Eg:/localdisk/skantama/tf-ngraph/tensorflow
    
    
    optional arguments:
    
    -h, --help show this help message and exit
    
    --list_tests LIST_TESTS
    
    Prints the list of test cases in this package.
    
    Eg:math_ops_test
    
    --run_test RUN_TEST Runs the testcase and returns the output.
    
    Eg:math_ops_test.DivNoNanTest.testBasic
    
    --run_tests_from_file RUN_TESTS_FROM_FILE
    
    Reads the test names specified in a file and runs
    
    them. Eg:--run_tests_from_file=tests_to_run.txt

  

## How to run tests

 - `--tensorflow_path` is a required argument and must be passed to
   specify the location of Tensorflow installation
   
 - Tests can be run by specifying one or multiple tests at a time by
   passing the name of the test or regular expressions.  Few examples of
   supported formats by `--run_test` argument :
 ``` math_ops_test 
       math_ops_test.DivNanTest
       math_ops_test.DivNoNanTest.testBasic
       math_ops_test.DivNoNanTest.*
       math_ops_test.D*
       math_ops_test.*
       math_*_test
       math_*_*_test
       math*_test
   ```
 -  List of tests to run can be listed in a text file and pass the file name 
     to  argument `--run_tests_from_file` to run. 

