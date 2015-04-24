#!/usr/bin/python3
import re
import subprocess
import os
import shlex

EXPECT_PATTERN = re.compile(r'// expect: \((.*)\)')

#Assuming that Makefile contains list of all unit-tests
os.system("make")

def get_contents(location):
    f = open(location)
    result = f.read()
    f.close()
    return result

def run_unit_test(unit_test):
    output=""
    try:
        output = subprocess.check_output(["bin/host_"+unit_test, "Hello World!"], stderr=subprocess.STDOUT, universal_newlines=True)
    except OSError:
        print("OSError")    #When running on non-epiphany system
    return output

def clean_str(string):
    return shlex.quote(string.replace("\n","\\n"))

def do_unit_test(unit_test):
    host_srctext = get_contents("./"+unit_test+"/host_"+unit_test+".c")
    host_expected_outputs = re.findall(EXPECT_PATTERN, host_srctext)

    e_srctext = get_contents("./"+unit_test+"/e_"+unit_test+".c")
    e_expected_outputs = re.findall(EXPECT_PATTERN, e_srctext)

    expected_output = "\n".join(host_expected_outputs)+"\n".join(e_expected_outputs);
    actual_output = run_unit_test(unit_test)
    
    succes = (actual_output == expected_output)
    
    if not succes:
        print('WARNING: Unit test "' + unit_test + '" failed')
        subprocess.call(['bash','-c','diff <(echo ' + clean_str(actual_output) + 
                    ') <(echo ' + clean_str(expected_output) + ')'])
        print("")
    return succes

maketext = get_contents("Makefile")
matches = re.search('all: (.*)\\n', maketext)
unit_tests = matches.group(1).split()

n_succes = 0
n_fail = 0
for unit_test in unit_tests:
    if do_unit_test(unit_test):
        n_succes += 1
    else:
        n_fail += 1

if n_fail == 0:
    print("All "+str(n_succes)+" tests where a succes!")
    print("Unit tests SUCCES")
else:
    print(str(n_fail)+" tests failed, "+str(n_succes)+" tests succeeded")
    print("Unit tests FAILURE")

