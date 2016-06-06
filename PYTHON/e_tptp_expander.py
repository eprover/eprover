#!/usr/bin/env python2.7

import sys, re, os

class TPTPExpander:

    COMMENTED_REGEX = "^\s*%"
    INCLUDE_REGEX = "^\s*include\s*\(\s*('|\")(.*)\\1\s*\)"

    @staticmethod
    def is_include(line):
        # If not commented ( Starts with zero or more whitespaces and then a percentage sign )
        if not TPTPExpander.is_commented(line):
            if re.match(TPTPExpander.INCLUDE_REGEX, line):
                return True
        return False

    @staticmethod
    def is_commented(line):
        if re.match(TPTPExpander.COMMENTED_REGEX, line) != None:
            return True
        return False

    @staticmethod
    def extract_filename(line):
        return re.match(TPTPExpander.INCLUDE_REGEX, line).group(2)

    @staticmethod
    def get_file_lines(filename):
        try:
            f = open(filename)
        except IOError as e:
            return None
        return [ line.rstrip("\n") for line in f ]

    @staticmethod
    def get_base_dir(filename):
        return os.path.dirname(os.path.realpath(filename))

    @staticmethod
    def expand_helper(lines, base_dir, doneFiles):
        if lines == None:
            return None
        ret = []
        for line in lines:
            if TPTPExpander.is_include(line):
                filename = TPTPExpander.extract_filename(line)
                if not os.path.isabs(filename):
                    filename = os.path.realpath(base_dir + os.path.sep + filename)
                if not doneFiles.get(filename,False):
                    doneFiles[filename] = True
                    tmp = TPTPExpander.expand_helper(TPTPExpander.get_file_lines(filename), TPTPExpander.get_base_dir(filename), doneFiles)
                    if tmp == None:
                        return None
                    else:
                        ret += tmp
            else:
                ret += [line]
        return ret

    @staticmethod
    def expand_from_array(lines, base_dir, is_to_string, base_done_dict = {}):
        tmp = TPTPExpander.expand_helper(lines, base_dir, base_done_dict)
        if tmp == None:
            return None
        elif is_to_string:
            return "\n".join(tmp)
        else:
            return tmp

    @staticmethod
    def expand_from_string(string, base_dir, is_to_string):
        return TPTPExpander.expand_from_array(string.split("\n"), base_dir, is_to_string)

    @staticmethod
    def expand_from_file(filename, is_to_string):
        if not os.path.isabs(filename):
            filename = os.path.realpath(os.getcwd() + os.path.sep + filename)
        return TPTPExpander.expand_from_array(TPTPExpander.get_file_lines(filename), TPTPExpander.get_base_dir(filename), is_to_string, {filename: True})

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "Usage : ./e_tptp_expander.py tptp_file"
        exit(1)
    res = TPTPExpander.expand_from_file(sys.argv[1], True)
    if res == None:
        print "Error: Failed reading one or more files"
        exit(1)
    else:
        print res
