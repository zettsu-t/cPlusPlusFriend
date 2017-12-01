#!/usr/bin/python3
# coding: utf-8

'''
This script extracts comments of a Python script and
checks whether it contains non-ASCII characters.
Copyright (C) 2017 Zettsu Tatsuya

usage : python3 comment_extractor.py [in-filename] > out-filename
'''

import re
import sys

EXIT_STATUS_SUCCESS = 0
EXIT_STATUS_ERROR = 1


class CommentExtractorState(object):
    '''Current state for a comment extractor'''

    def __init__(self):
        self.exit_status = EXIT_STATUS_SUCCESS
        self.line_header = ''
        self.all_comments = ''
        self.leading_comment = None
        self.matcher = None
        self.is_comment = False


class CommentExtractor(object):
    '''Extract comments in Python scripts'''

    def __init__(self, command_line_arguments):
        self.filenames = command_line_arguments[1:]

    def parse(self):
        '''Extract comments in Python scripts'''

        status = EXIT_STATUS_SUCCESS
        for filename in self.filenames:
            if self.parse_file(filename):
                status = EXIT_STATUS_ERROR

        return status

    def parse_file(self, filename):
        '''Extract comments in a Python script'''

        exit_status = EXIT_STATUS_SUCCESS
        try:
            with open(filename, 'r') as infile:
                all_comments, status = self.parse_lines(
                    sys.stderr, infile, filename)
                print(all_comments)
                if status is not EXIT_STATUS_SUCCESS:
                    exit_status = status
        except IOError:
            message = 'Cannot open {}\n'.format(filename)
            sys.stderr.write(message)
            exit_status = EXIT_STATUS_ERROR

        return exit_status

    @staticmethod
    def find_delimiter(line, delimiter):
        '''Finds a delimiter in a line and splits the line by the delimiter'''

        position = line.find(delimiter)
        matcher = None
        prefix = ''
        tail = ''
        prechar = line[position - 1:position] if position > 0 else ''

        if position >= 0 and prechar != '\\':
            if delimiter == '/*':
                matcher = r'(.*?)((\*\/))(.*)'
            else:
                matcher = r'(.*?)((%s){1}?)(.*)' % delimiter
            prefix = line[0:position]
            tail = line[position + len(delimiter):]
        else:
            tail = line
        return matcher, prefix, tail

    def extract_delimiter(self, line):
        '''
        Finds one of the delimiters in a line and splits the line by the
        delimiter
        '''

        delimiters = ["'''", '"""', '/*', "'", '"']
        results = [(self.find_delimiter(line, delimiter), delimiter) \
                   for delimiter in delimiters]

        matcher = None
        prefix = ''
        tail = line
        is_comment = False
        for (matcher_d, prefix_d, tail_d), delimiter in results:
            if matcher_d is not None:
                if matcher is None or len(prefix_d) < len(prefix):
                    matcher = matcher_d
                    prefix = prefix_d
                    tail = tail_d
                    is_comment = len(delimiter) > 1

        return matcher, tail, is_comment

    def extract_comments_in_line(self, line, leading_comment, matcher, is_comment):
        '''
        Extracts comments in a line and returns collected comments
        and a comment which continues to the next line.
        '''

        tail = ''
        comments = []
        comment_to_next = ''

        if matcher is None:
            matcher, tail, is_comment = self.extract_delimiter(line)
            if matcher is None:
                match_obj = re.match(r'^[^#]*(#+)\s*(.*)', line)
                if match_obj is None:
                    match_obj = re.match(r'^(.*?)\/\/\s*(.*)', line)
                if match_obj is not None:
                    tail_comment = match_obj.group(2).strip()
                    # Separate comments from previous lines
                    if tail_comment:
                        if leading_comment:
                            comments.append('')
                        comments.append(tail_comment)
                tail = ''
        else:
            match_obj = re.match(matcher, line)
            if match_obj is None:
                comment_to_next = line.strip()
            else:
                if is_comment:
                    phrase = match_obj.group(1).strip()
                    if phrase:
                        comments.append(phrase)
                tail = match_obj.group(4)
                matcher = None

        return tail, comments, comment_to_next, matcher, is_comment

    @staticmethod
    def is_ascll_only(errstream, filename, line_number, line, old_exit_status):
        '''checks whether the line contains US-ASCII characters only'''

        exit_status = old_exit_status
        if not all(ord(c) < 128 for c in line):
            message = 'Non US-ASCII character found at line {} in {}\n'. \
                format(line_number, filename)
            errstream.write(message)
            exit_status = EXIT_STATUS_ERROR

        return exit_status

    @staticmethod
    def format_comments(comments, leading_comment, line_header, current_line_header):
        '''Extracts multiple comments in a line'''

        if not comments:
            return '', leading_comment, line_header

        comment_string = ''
        start_position = 0

        comment_string += line_header
        if leading_comment is not None:
            space = ' ' if leading_comment and comments[0] else ''
            comment_string += leading_comment + space + comments[0] + '\n\n'
            start_position = 1

        trailing_comments = comments[start_position:]
        if line_header is not current_line_header and trailing_comments:
            comment_string += current_line_header

        for phrase in trailing_comments:
            comment_string += phrase + '\n\n'

        return comment_string, None, ''

    @staticmethod
    def format_comment_to_next(comment_to_next, leading_comment, matcher, line_header):
        '''Extracts a comment in consecutive lines'''

        comment_string = ''
        if comment_to_next:
            if leading_comment is None:
                leading_comment = comment_to_next
            else:
                prefix = ' ' if leading_comment else ''
                leading_comment += prefix + comment_to_next
        else:
            if leading_comment is not None and leading_comment:
                comment_string = line_header + leading_comment + '\n\n'
                leading_comment = None
            if matcher is None:
                line_header = ''

        return comment_string, leading_comment, line_header

    @staticmethod
    def warn_io_error(errstream, line_number, filename):
        '''Warns I/O errors'''
        message = 'Cannot read at line {} in {}\n'. \
                  format(line_number, filename)
        errstream.write(message)

    def parse_line(self, line, errstream, line_number, filename, state):
        '''Parse a line and collect comments'''
        current_line_header = '{0} : {1}\n'.format(filename, line_number)
        line_header = state.line_header if state.line_header else current_line_header
        exit_status = self.is_ascll_only(errstream, filename, line_number, line, state.exit_status)

        hash_comment = re.match(r'^\s*#\s*(.*)', line)
        if hash_comment is not None:
            newline = ''
            comments = []
            comment_to_next = hash_comment.group(1)
            matcher = None
            is_comment = True

        while line:
            if hash_comment is None:
                newline, comments, comment_to_next, matcher, is_comment = self.extract_comments_in_line(
                    line, state.leading_comment, state.matcher, state.is_comment)
            line = newline

            comment_string, leading_comment, line_header = self.format_comments(
                comments, state.leading_comment, line_header, current_line_header)
            state.all_comments += comment_string

            comment_string, leading_comment, line_header = self.format_comment_to_next(
                comment_to_next, leading_comment, matcher, line_header)
            state.all_comments += comment_string

            state.exit_status = exit_status
            state.line_header = line_header
            state.leading_comment = leading_comment
            state.matcher = matcher
            state.is_comment = is_comment

        return state

    def parse_lines(self, errstream, infile, filename):
        '''Extracts a comment in all lines in the file'''

        line_number = 0
        state = CommentExtractorState()

        try:
            line = infile.readline()
            while line:
                line_number += 1
                state = self.parse_line(line, errstream, line_number, filename, state)
                line = infile.readline()
                if not line and line_number == 1:
                    line = '\n'

        except IOError:
            self.warn_io_error(sys.stderr, line_number, filename)
            state.exit_status = EXIT_STATUS_ERROR

        return [state.all_comments, state.exit_status]

if __name__ == "__main__":
    sys.exit(CommentExtractor(sys.argv).parse())
