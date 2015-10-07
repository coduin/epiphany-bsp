#!/usr/bin/python3
import re
import subprocess
import os
import shlex
import sys
import difflib

NPROCS = 16
EXPECT_FOR_PID_PATTERN = re.compile(r'// expect_for_pid: \((.*)\)')
EXPECT_PATTERN = re.compile(r'// expect: \((.*)\)')

#Assuming that Makefile contains list of all unit-tests

def expand_fn(mo):
    result = ""
    for pid in range(0, NPROCS):
        result += "// expect: (${0:02d}: {1})\n".format(pid, eval(mo.group(1)))
    return result

def expand_pid_pattern(content):
    output = ""
    for line in content.split('\n'):
        output += re.sub(EXPECT_FOR_PID_PATTERN, expand_fn, line)
        output += "\n"
    return output

def get_contents(location):
    f = open(location)
    result = f.read()
    f.close()
    return result

def run_unit_test(unit_test):
    output=""
    try:
        output = subprocess.check_output(["bin/host_"+unit_test, "Hello World!"], stderr=subprocess.STDOUT, universal_newlines=True, timeout=5)
    except OSError:
        print("OSError")    #When running on non-epiphany systems
    except subprocess.TimeoutExpired:
        return "TIMEOUT"
    return output

def clean_str(string):
    return string #shlex.quote(string.replace("\n","\\n"))

def do_unit_test(unit_test):
    host_srctext = get_contents("./"+unit_test+"/host_"+unit_test+".c")
    host_srctext = expand_pid_pattern(host_srctext)
    host_expected_outputs = re.findall(EXPECT_PATTERN, host_srctext)

    e_srctext = get_contents("./"+unit_test+"/e_"+unit_test+".c")
    e_srctext = expand_pid_pattern(e_srctext)
    e_expected_outputs = re.findall(EXPECT_PATTERN, e_srctext)

    expected_output = "\n".join(e_expected_outputs)
    if len(host_expected_outputs) != 0:
        expected_output += "\n" + "\n".join(host_expected_outputs);
    actual_output = run_unit_test(unit_test)
    
    succes = (actual_output.replace("\n","") == expected_output.replace("\n",""))
    
    if not succes:
        print('WARNING: Unit test "' + unit_test + '" failed with output:')
        if actual_output == "TIMEOUT":
            print("A timeout occurred")
        else:
            for line in difflib.context_diff(actual_output.split('\n'), expected_output.split('\n')):
                print(line)
    return succes

maketext = get_contents("Makefile")
matches = re.search('tests: (.*)\\n', maketext)
unit_tests = matches.group(1).split()

n_succes = 0
n_fail = 0
for unit_test in unit_tests:
    print("Testing "+unit_test+"...")
    if do_unit_test(unit_test):
        n_succes += 1
    else:
        n_fail += 1

if n_fail == 0:
    print("All "+str(n_succes)+" tests were succesful!")
    print("--- SUCCES")
else:
    print(str(n_fail)+" tests failed, "+str(n_succes)+" tests succeeded")
    print("--- FAILED")

