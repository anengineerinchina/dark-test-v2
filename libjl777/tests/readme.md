BDD testing with Cucumber
-------------------------

Description

In BDD the test cases must be human readable so when a test case is broken it should be opbvious what is the featuer and the bussiness value lost
The framework uses HTTP requests to interact with superNET program and the console output log to verify the expected outcomes


Installation

Enter in the tests folder and execute ./setup.sh (this should install all required dependencies)


Usage

Pre-requeriment: The first time the tests are executed, the test framework needs to launch BitcoinDarkd to capture its output, so make sure it's stopped with this command:
./BitcoinDarkd stop

Note: after the first execution, if the program is not stopped, there is no need to stop it to run the tests again

Execution: From the tests folder just type the command below to execute all tests:
cucumber

To execute specific tests use the parameter --tags and the name of some tags specified in the feature files. Exmaple:
cucumber --tags @general,@send_message

Output: The output of the tests is displayed on the console, to show report on the console and a nice html report, use:
cucumber -f pretty -f html --out report.html
