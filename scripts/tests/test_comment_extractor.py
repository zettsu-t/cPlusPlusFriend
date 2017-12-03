#!/usr/bin/python3
# coding: utf-8

'''
This script tests the comment extractor comment_extractor.py
Copyright (C) 2017 Zettsu Tatsuya

usage : python3 -m unittest discover tests
'''

import re
from unittest import TestCase
import comment_extractor.comment_extractor as tested


class MockInputStream(object):
    '''Does like a file input stream'''

    def __init__(self, lines):
        self.lines = lines

    def readline(self):
        '''Read a preset line'''

        if not self.lines:
            return ''
        line = self.lines.pop(0)
        return line + '\n'


class MockOutputStream(object):
    '''Does like stderr'''

    def __init__(self):
        self.message = ''

    def write(self, message):
        '''Store a line'''

        self.message += message


class TestFindDelimiter(TestCase):
    '''Testing find_delimiter'''

    def test_find_delimiter_no_single_quote(self):
        '''Single quotes'''

        delimiter = "'''"
        for line in ['', "'", "''", "\\'''"]:
            matcher, prefix, tail = tested.CommentExtractor. \
                                    find_delimiter(line, delimiter)
            self.assertIsNone(matcher)
            self.assertEqual(prefix, '')
            self.assertEqual(tail, line)

    def test_find_delimiter_no_double_quote(self):
        '''Double quotes'''

        delimiter = '"""'
        for line in ['', '"', '""', '\\"""']:
            matcher, prefix, tail = tested.CommentExtractor. \
                                    find_delimiter(line, delimiter)
            self.assertIsNone(matcher)
            self.assertEqual(prefix, '')
            self.assertEqual(tail, line)

    def test_find_matcher_valid(self):
        '''Comments betweeen two triple quotes'''
        for delimiter in ["'''", '"""']:
            for expected_prefix in ['', ' ', 'a', 'ab', "ab3"]:
                expected_tail = 'POST'
                line = expected_prefix + delimiter + expected_tail
                matcher, prefix, tail = tested.CommentExtractor. \
                                        find_delimiter(line, delimiter)
                self.assertIsNotNone(matcher)
                self.assertEqual(prefix, expected_prefix)
                self.assertEqual(tail, expected_tail)

    def test_find_matcher_c_style(self):
        '''C-style comments'''
        line = 'pre /* comment */ post'
        matcher, prefix, tail = tested.CommentExtractor. \
                                find_delimiter(line, '/*')
        self.assertIsNotNone(matcher)
        self.assertEqual(prefix, 'pre ')
        self.assertEqual(tail, ' comment */ post')

        matched_object = re.match(matcher, tail)
        self.assertIsNotNone(matched_object)
        self.assertEqual(matched_object.group(2), '*/')
        self.assertEqual(matched_object.group(4), ' post')

    def test_find_matcher_reluctant(self):
        '''Find first delimiters with a reluctant regexp'''
        for delimiter in ["'''", '"""']:
            expected_prefix = 'PRE'
            expected_post = 'POST'
            comment = 'COMMENT'
            expected_tail = comment + delimiter + expected_post
            line = expected_prefix + delimiter + expected_tail
            matcher, prefix, tail = tested.CommentExtractor. \
                                    find_delimiter(line, delimiter)
            self.assertIsNotNone(matcher)
            self.assertEqual(prefix, expected_prefix)
            self.assertEqual(tail, expected_tail)

            matched_obj = re.match(matcher, expected_tail)
            self.assertIsNotNone(matched_obj)
            self.assertEqual(matched_obj.group(1), comment)
            self.assertEqual(matched_obj.group(2), delimiter)
            self.assertEqual(matched_obj.group(3), delimiter)
            self.assertEqual(matched_obj.group(4), expected_post)


class TestExtractDelimiter(TestCase):
    '''Testing extract_delimiter'''

    def test_extract_delimiter_invalid(self):
        '''Cannot find delimiters'''
        extractor = tested.CommentExtractor([])
        for line in ['', 'a']:
            matcher, tail, is_comment = extractor. \
                                        extract_delimiter(line)
            self.assertIsNone(matcher)
            self.assertEqual(tail, line)
            self.assertFalse(is_comment)

    def test_extract_delimiter_string1(self):
        '''Find string literals including quotation marks'''
        extractor = tested.CommentExtractor([])

        cases = [["'", ''],
                 ["pre'string", 'string'],
                 ['pre"string', 'string'],
                 ['"' + "'''", "'''"],
                 ['+ "' + "'''" + '" +', "'''" + '" +'],
                 ["'" + '"""', '"""'],
                 ["'" + '"""' + "'", '"""' + "'"]]
        for line, expected_tail in cases:
            matcher, tail, is_comment = extractor. \
                                                extract_delimiter(line)
            self.assertIsNotNone(matcher)
            self.assertEqual(tail, expected_tail)
            self.assertFalse(is_comment)

    def test_extract_delimiter_string2(self):
        '''Find string literals'''
        extractor = tested.CommentExtractor([])

        for delimiter in ["'", '"']:
            expected_prefix = 'PRE'
            expected_post = 'POST'
            comment = 'STRING'
            line = expected_prefix + delimiter + comment
            line += delimiter + expected_post
            matcher, tail, is_comment = extractor.extract_delimiter(line)
            self.assertIsNotNone(matcher)
            self.assertFalse(is_comment)

            matched_obj = re.match(matcher, tail)
            self.assertIsNotNone(matched_obj)
            self.assertEqual(matched_obj.group(1), comment)
            self.assertEqual(matched_obj.group(2), delimiter)
            self.assertEqual(matched_obj.group(3), delimiter)
            self.assertEqual(matched_obj.group(4), expected_post)

    def test_extract_delimiter_comment1(self):
        '''Find string literals including triple single-quotation marks'''
        cases = [["'''", ''],
                 ["pre'''comment", 'comment'],
                 ["pre'''comment'''post", "comment'''post"],
                 ['pre"""comment', 'comment'],
                 ['pre"""comment"""post', 'comment"""post'],
                 ['pre"""comment' + "'''post", "comment'''post"],
                 ["pre'''comment" + '"""post', 'comment"""post']]

        extractor = tested.CommentExtractor([])
        for line, expected_tail in cases:
            matcher, tail, is_comment = extractor.extract_delimiter(line)
            self.assertIsNotNone(matcher)
            self.assertEqual(tail, expected_tail)
            self.assertTrue(is_comment)
        extractor = tested.CommentExtractor([])

    def test_extract_delimiter_comment2(self):
        '''Find string literals including triple double-quotation marks'''
        extractor = tested.CommentExtractor([])
        line = "pre'" + '1"""2' + "'post"
        matcher, tail, is_comment = extractor.extract_delimiter(line)
        self.assertIsNotNone(matcher)
        self.assertFalse(is_comment)

        matched_obj = re.match(matcher, tail)
        self.assertIsNotNone(matched_obj)
        self.assertEqual(matched_obj.group(1), '1"""2')
        self.assertEqual(matched_obj.group(2), "'")
        self.assertEqual(matched_obj.group(3), "'")
        self.assertEqual(matched_obj.group(4), 'post')

    def test_extract_comments_hatch(self):
        '''Find comments after #'''
        cases = [['', None, []],
                 ['#word ', None, ['word']],
                 ['# word ', None, ['word']],
                 ['## word ', None, ['word']],
                 ['### word ', None, ['word']],
                 ['## # of e', None, ['# of e']],
                 ['code # word', None, ['word']],
                 ['code # word', 'a', ['', 'word']]]
        extractor = tested.CommentExtractor([])
        for line, leading_comment, expected_comments in cases:
            matcher = None
            is_comment = False
            tail = line
            all_comments = []
            for _ in range(10):
                tail, comments, _, matcher, is_comment = \
                    extractor.extract_comments_in_line(tail, leading_comment, matcher, is_comment)
                all_comments += comments
                if not tail:
                    break

            self.assertEqual(all_comments, expected_comments)
            self.assertIsNone(matcher)

    def test_extract_comments_double_slash(self):
        '''Find comments after //'''
        cases = [['', None, []],
                 ['// word ', None, ['word']],
                 ['code // word', None, ['word']],
                 ['code // 1st // 2nd', 'a', ['', '1st // 2nd']]]
        extractor = tested.CommentExtractor([])
        for line, leading_comment, expected_comments in cases:
            matcher = None
            is_comment = False
            tail = line
            all_comments = []
            for _ in range(10):
                tail, comments, _, matcher, is_comment = \
                    extractor.extract_comments_in_line(tail, leading_comment, matcher, is_comment)
                all_comments += comments
                if not tail:
                    break

            self.assertEqual(all_comments, expected_comments)
            self.assertIsNone(matcher)

    def test_extract_comments_triple(self):
        '''Find comments after triple double-quotation marks'''
        cases = [['"""word', 'word'],
                 ["code ''' word", ' word']]
        extractor = tested.CommentExtractor([])
        for line, expected_tail in cases:
            tail, comments, comment, matcher, is_comment = \
                extractor.extract_comments_in_line(line, None, None, False)
            self.assertEqual(tail, expected_tail)
            self.assertEqual(comments, [])
            self.assertEqual(comment, '')
            self.assertIsNotNone(matcher)
            self.assertTrue(is_comment)

    def test_extract_comments_string_literal(self):
        '''Find string literals'''
        cases = [['"word"', 'word"'],
                 ["pre, 'word' post", "word' post"],
                 ['"' + "'''" + '"', "'''" +  '"'],
                 ["'" + '"""' + "'", '"""' + "'"]]
        extractor = tested.CommentExtractor([])
        for line, expected_tail in cases:
            tail, comments, comment, matcher, is_comment = \
                extractor.extract_comments_in_line(line, None, None, False)
            self.assertEqual(tail, expected_tail)
            self.assertEqual(comments, [])
            self.assertEqual(comment, '')
            self.assertIsNotNone(matcher)
            self.assertFalse(is_comment)

    def test_extract_comments_mixed(self):
        '''Find comments and string literals'''
        cases = [['"""word""" "string"', ['word']],
                 ["code 'string' # word", ['word']],
                 ["code 'string' '''a''' # word", ['a', 'word']],
                 ['"""a"""' + "'string'" + '"string"' + "'''word'''", ['a', 'word']]]
        extractor = tested.CommentExtractor([])
        for line, expected_comments in cases:
            matcher = None
            is_comment = False
            tail = line
            all_comments = []
            for _ in range(10):
                tail, comments, _, matcher, is_comment = \
                    extractor.extract_comments_in_line(tail, None, matcher, is_comment)
                all_comments += comments
                if not tail:
                    break

            self.assertEqual(all_comments, expected_comments)

    def test_extract_multiline1(self):
        '''Find comments in lines beginning with triple single-quotation marks'''
        extractor = tested.CommentExtractor([])
        matcher = None
        is_comment = False
        all_comments = []

        tail = "'''1st\n"
        for _ in range(10):
            tail, comments, _, matcher, is_comment = \
                extractor.extract_comments_in_line(tail, None, matcher, is_comment)
            all_comments += comments
            if not tail:
                break

        self.assertEqual(comments, [])
        self.assertIsNotNone(matcher)
        self.assertTrue(is_comment)

        tail = "2nd line\n"
        extractor = tested.CommentExtractor([])
        for _ in range(10):
            tail, comments, _, matcher, is_comment = \
                extractor.extract_comments_in_line(tail, '1st', matcher, is_comment)
            all_comments += comments
            if not tail:
                break
        self.assertEqual(comments, [])
        self.assertIsNotNone(matcher)
        self.assertTrue(is_comment)

        tail = "'string'\n"
        extractor = tested.CommentExtractor([])
        for _ in range(10):
            tail, comments, _, matcher, is_comment = \
                extractor.extract_comments_in_line(tail, '1st 2nd line', matcher, is_comment)
            all_comments += comments
            if not tail:
                break
        self.assertEqual(comments, [])
        self.assertIsNotNone(matcher)
        self.assertTrue(is_comment)

        tail = "last''' code # comment\n"
        extractor = tested.CommentExtractor([])
        for _ in range(10):
            tail, comments, _, matcher, is_comment = \
                extractor.extract_comments_in_line(tail, '1st 2nd line string', matcher, is_comment)
            all_comments += comments
            if not tail:
                break
        self.assertEqual(all_comments, ['last', '', 'comment'])
        self.assertIsNone(matcher)

    def test_extract_comments_multiline2(self):
        '''Find comments in lines beginning with triple double-quotation marks'''
        extractor = tested.CommentExtractor([])
        matcher = None
        is_comment = False
        all_comments = []
        comment = ''

        tail = '"""1st\n'
        for _ in range(10):
            tail, comments, comment, matcher, is_comment = \
                extractor.extract_comments_in_line(tail, None, matcher, is_comment)
            all_comments += comments
            if not tail:
                break
        self.assertEqual(comments, [])
        self.assertEqual(comment, '1st')
        self.assertIsNotNone(matcher)
        self.assertTrue(is_comment)

        tail = '2nd line""" "string" """next\n'
        for _ in range(10):
            tail, comments, comment, matcher, is_comment = \
                extractor.extract_comments_in_line(tail, '1st', matcher, is_comment)
            all_comments += comments
            if not tail:
                break
        self.assertEqual(all_comments, ['2nd line'])
        self.assertEqual(comment, 'next')
        self.assertIsNotNone(matcher)
        self.assertTrue(is_comment)

    def test_extract_comments_empty_multiline(self):
        '''Find empty comments'''
        for empty_lines in range(4):
            extractor = tested.CommentExtractor([])
            line = '"""'
            tail, comments, comment, matcher, is_comment = extractor.extract_comments_in_line(line, None, None, False)
            self.assertEqual(tail, '')
            self.assertEqual(comments, [])
            self.assertEqual(comment, '')
            self.assertIsNotNone(matcher)
            self.assertTrue(is_comment)

            for _ in range(empty_lines):
                line = ''
                extractor = tested.CommentExtractor([])
                tail, comments, comment, matcher, is_comment = \
                    extractor.extract_comments_in_line(line, None, matcher, is_comment)
                self.assertEqual(tail, '')
                self.assertEqual(comments, [])
                self.assertEqual(comment, '')
                self.assertIsNotNone(matcher)
                self.assertTrue(is_comment)

            line = '"""'
            extractor = tested.CommentExtractor([])
            tail, comments, comment, matcher, is_comment = \
                extractor.extract_comments_in_line(line, None, matcher, is_comment)
            self.assertEqual(tail, '')
            self.assertEqual(comments, [])
            self.assertEqual(comment, '')
            self.assertIsNone(matcher)
            self.assertTrue(is_comment)


class TestCheckCharacters(TestCase):
    '''Testing is_ascll_only'''

    def test_is_ascll_only(self):
        '''Check whether lines contain US-ASCII only'''
        cases = [['', tested.EXIT_STATUS_SUCCESS,
                  '', tested.EXIT_STATUS_SUCCESS],
                 ['', tested.EXIT_STATUS_ERROR,
                  '', tested.EXIT_STATUS_ERROR,],
                 ['word 1', tested.EXIT_STATUS_SUCCESS,
                  '', tested.EXIT_STATUS_SUCCESS],
                 ['word 2', tested.EXIT_STATUS_ERROR,
                  '', tested.EXIT_STATUS_ERROR],
                 ['単語', tested.EXIT_STATUS_SUCCESS,
                  'Non US-ASCII character found at line 23 in F\n',
                  tested.EXIT_STATUS_ERROR],
                 ['単語', tested.EXIT_STATUS_ERROR,
                  'Non US-ASCII character found at line 23 in F\n',
                  tested.EXIT_STATUS_ERROR]]
        extractor = tested.CommentExtractor([])

        for line, old_status, expected_message, expected_status in cases:
            errstream = MockOutputStream()
            new_status = extractor.is_ascll_only(errstream, "F", 23, line, old_status)
            self.assertEqual(errstream.message, expected_message)
            self.assertEqual(new_status, expected_status)


class TestFormatComment(TestCase):
    '''Testing format_comments'''

    def test_format_comments_empty(self):
        '''Format empty comments'''
        extractor = tested.CommentExtractor([])
        comment_string, leading_comment, line_header = extractor.format_comments(
            [], 'comment', 'line 1:', 'line 2:')

        self.assertEqual(comment_string, '')
        self.assertEqual(leading_comment, 'comment')
        self.assertEqual(line_header, 'line 1:')

    def test_format_comments_not_empty(self):
        '''Format non-empty comments'''
        cases = [[[''], '', 'line 1:', 'line 2:',
                  'line 1:\n\n'],
                 [['first'], '', 'line 1:', 'line 2:',
                  'line 1:first\n\n'],
                 [['second'], 'first', 'line 1:', 'line 2:',
                  'line 1:first second\n\n'],
                 [['second', 'third'], 'first', 'line 1:', 'line 2:',
                  'line 1:first second\n\nline 2:third\n\n'],
                 [[''], 'first', 'line 1:', 'line 2:',
                  'line 1:first\n\n'],
                 [['second'], 'first', 'line 1:', 'line 1:',
                  'line 1:first second\n\n'],
                 [['second'], 'first', 'line 1:', 'line 2:',
                  'line 1:first second\n\n'],
                 [['second', 'third'], 'first', 'line 1:', 'line 2:',
                  'line 1:first second\n\nline 2:third\n\n']]
        extractor = tested.CommentExtractor([])

        for comments, leading_comment, line_header, current_line_header, \
            expected_comment_string in cases:
            comment_string, leading_comment, line_header = extractor.format_comments(
                comments, leading_comment, line_header, current_line_header)
            self.assertEqual(comment_string, expected_comment_string)
            self.assertIsNone(leading_comment)
            self.assertEqual(line_header, '')

    def test_format_comment_to_next_new(self):
        '''Format comments leading to its next line'''
        cases = [[None, None, '',
                  '', None, ''],
                 ['', None, '',
                  '', '', ''],
                 ['leading', None, 'line 1:',
                  'line 1:leading\n\n', None, ''],
                 ['leading', 1, 'line 1:',
                  'line 1:leading\n\n', None, 'line 1:']]
        extractor = tested.CommentExtractor([])

        for leading_comment, matcher, line_header, \
            expected_comment_string, expected_leading_comment, expected_line_header in cases:
            comment_string, leading_comment, line_header = extractor.format_comment_to_next(
                '', leading_comment, matcher, line_header)
            self.assertEqual(comment_string, expected_comment_string)
            self.assertEqual(leading_comment, expected_leading_comment)
            self.assertEqual(line_header, expected_line_header)

    def test_format_comment_to_next_continued(self):
        '''Format comments followed by its previous line'''
        cases = [[None, None, 'next'],
                 ['', None, 'next'],
                 ['first', None, 'first next']]
        extractor = tested.CommentExtractor([])

        for leading_comment, matcher, expected_leading_comment in cases:
            comment_string, leading_comment, line_header = extractor.format_comment_to_next(
                'next', leading_comment, matcher, 'line 1:')
            self.assertEqual(comment_string, '')
            self.assertEqual(leading_comment, expected_leading_comment)
            self.assertEqual(line_header, 'line 1:')


class TestWarnIO(TestCase):
    '''Testing warn_io_error'''

    def test_warn_io_error(self):
        '''Warn if I/O errors occur'''
        extractor = tested.CommentExtractor([])
        errstream = MockOutputStream()
        extractor.warn_io_error(errstream, 34, 'F')
        self.assertEqual(errstream.message, 'Cannot read at line 34 in F\n')


class TestParasLine(TestCase):
    '''Testing parse_line'''

    def test_parse_line_multiline_quotes(self):
        '''Parse a comment in multiple lines'''
        errstream = MockOutputStream()
        state = tested.CommentExtractorState()
        filename = 'F'
        extractor = tested.CommentExtractor([])

        cases = [['',
                  '', '', None, True, False],
                 ["'''",
                  'F : 2\n', '', None, False, True],
                 ['First', 'F : 2\n', '', 'First', False, True],
                 ['Second', 'F : 2\n', '', 'First Second', False, True],
                 ["'''", '', 'F : 2\nFirst Second\n\n', None, True, True]]
        extractor = tested.CommentExtractor([])

        line_number = 0
        for line, expected_line_header, expected_comments, expected_leading_comment, \
            expected_matcher, expected_is_comment in cases:
            line_number += 1
            state = extractor.parse_line(line, errstream, line_number, filename, state)
            self.assertEqual(errstream.message, '')
            self.assertEqual(state.exit_status, tested.EXIT_STATUS_SUCCESS)
            self.assertEqual(state.line_header, expected_line_header)
            self.assertEqual(state.all_comments, expected_comments)

            if expected_leading_comment is None:
                self.assertIsNone(state.leading_comment)
            else:
                self.assertIsNotNone(state.leading_comment)
                self.assertEqual(state.leading_comment, expected_leading_comment)

            if expected_matcher:
                self.assertIsNone(state.matcher)
            else:
                self.assertIsNotNone(state.matcher)

            self.assertEqual(state.is_comment, expected_is_comment)

    def test_parse_line_multiline_hashes(self):
        '''Concatenate comments in multiple #... lines to one'''
        errstream = MockOutputStream()
        state = tested.CommentExtractorState()
        filename = 'F'
        extractor = tested.CommentExtractor([])

        cases = [['',
                  '', '', None, True, False],
                 ['# first',
                  'F : 2\n', '', 'first', True, True],
                 ['# second', 'F : 2\n', '', 'first second', True, True],
                 ['code #third', '', 'F : 2\nfirst second\n\nF : 4\nthird\n\n', None, True, False]]
        extractor = tested.CommentExtractor([])

        line_number = 0
        for line, expected_line_header, expected_comments, expected_leading_comment, \
            expected_matcher, expected_is_comment in cases:
            line_number += 1
            state = extractor.parse_line(line, errstream, line_number, filename, state)
            self.assertEqual(errstream.message, '')
            self.assertEqual(state.exit_status, tested.EXIT_STATUS_SUCCESS)
            self.assertEqual(state.line_header, expected_line_header)
            self.assertEqual(state.all_comments, expected_comments)

            if expected_leading_comment is None:
                self.assertIsNone(state.leading_comment)
            else:
                self.assertIsNotNone(state.leading_comment)
                self.assertEqual(state.leading_comment, expected_leading_comment)

            if expected_matcher:
                self.assertIsNone(state.matcher)
            else:
                self.assertIsNotNone(state.matcher)

            self.assertEqual(state.is_comment, expected_is_comment)

    def test_parse_line_multiline_mixed(self):
        '''Parse comments in mixed style'''
        errstream = MockOutputStream()
        state = tested.CommentExtractorState()
        filename = 'F'
        extractor = tested.CommentExtractor([])

        cases = [['"""1st',
                  'F : 1\n', '', '1st', False, True],
                 ['2nd',
                  'F : 1\n', '', '1st 2nd', False, True],
                 ['3rd""" # 4th', '', 'F : 1\n1st 2nd 3rd\n\nF : 3\n4th\n\n', None, True, False],
                 ['code # 5th', '', 'F : 1\n1st 2nd 3rd\n\nF : 3\n4th\n\nF : 4\n5th\n\n', None, True, False]]
        extractor = tested.CommentExtractor([])

        line_number = 0
        for line, expected_line_header, expected_comments, expected_leading_comment, \
            expected_matcher, expected_is_comment in cases:
            line_number += 1
            state = extractor.parse_line(line, errstream, line_number, filename, state)
            self.assertEqual(errstream.message, '')
            self.assertEqual(state.exit_status, tested.EXIT_STATUS_SUCCESS)
            self.assertEqual(state.line_header, expected_line_header)
            self.assertEqual(state.all_comments, expected_comments)

            if expected_leading_comment is None:
                self.assertIsNone(state.leading_comment)
            else:
                self.assertIsNotNone(state.leading_comment)
                self.assertEqual(state.leading_comment, expected_leading_comment)

            if expected_matcher:
                self.assertIsNone(state.matcher)
            else:
                self.assertIsNotNone(state.matcher)

            self.assertEqual(state.is_comment, expected_is_comment)

    def test_parse_line_empty_multiline(self):
        '''Parse empty comments in multiple lines'''
        errstream = MockOutputStream()
        state = tested.CommentExtractorState()
        filename = 'F'
        extractor = tested.CommentExtractor([])

        cases = [['',
                  '', '', None, True, False],
                 ["'''",
                  'F : 2\n', '', None, False, True],
                 ['', 'F : 2\n', '', None, False, True],
                 ["'''", '', '', None, True, True]]
        extractor = tested.CommentExtractor([])

        line_number = 0
        for line, expected_line_header, expected_comments, expected_leading_comment, \
            expected_matcher, expected_is_comment in cases:
            line_number += 1
            state = extractor.parse_line(line, errstream, line_number, filename, state)
            self.assertEqual(errstream.message, '')
            self.assertEqual(state.exit_status, tested.EXIT_STATUS_SUCCESS)
            self.assertEqual(state.line_header, expected_line_header)
            self.assertEqual(state.all_comments, expected_comments)

            if expected_leading_comment is None:
                self.assertIsNone(state.leading_comment)
            else:
                self.assertIsNotNone(state.leading_comment)
                self.assertEqual(state.leading_comment, expected_leading_comment)

            if expected_matcher:
                self.assertIsNone(state.matcher)
            else:
                self.assertIsNotNone(state.matcher)

            self.assertEqual(state.is_comment, expected_is_comment)

    def test_parse_lines(self):
        '''Parse complex cases for comments'''
        cases = [[[''], ''],
                 [['#comment'], 'Source : 1\ncomment\n\n'],
                 [['', '#1st', '#2nd', '', '#3rd', '', '#4th'], 'Source : 2\n1st 2nd\n\nSource : 5\n3rd\n\n'],
                 [['', '"""comment"""'], 'Source : 2\ncomment\n\n'],
                 [['', "'''", 'comment', "'''"], 'Source : 2\ncomment\n\n'],
                 [['', "'''a", 'b', "c'''"], 'Source : 2\na b c\n\n'],
                 [['', "'''a", 'b', "c''' # d"], 'Source : 2\na b c\n\nSource : 4\nd\n\n'],
                 [['', "'''a", 'b', "c''' '''d'''"], 'Source : 2\na b c\n\nSource : 4\nd\n\n'],
                 [['', '/*', '1st', '2nd', "*/ '''d'''"], 'Source : 2\n1st 2nd\n\nSource : 5\nd\n\n']]
        extractor = tested.CommentExtractor([])

        for lines, expected in cases:
            errstream = MockOutputStream()
            comments, status = extractor.parse_lines(errstream, MockInputStream(lines), 'Source')
            self.assertEqual(comments, expected)
            self.assertEqual(status, tested.EXIT_STATUS_SUCCESS)
