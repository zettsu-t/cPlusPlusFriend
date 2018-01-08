#!/usr/bin/python3
# coding: utf-8

'''
Convert hashtags
'''

import codecs
import csv
import sys

HASH_TAGS = {'#newgame':'NewGame',
             '#けものフレンズ':'KemonoFriends',
             '#やめるのだフェネックで学ぶ':'StopFennec'}

def convert_file(infilename, outfilename):
    '''
    Convert hashtags
    '''

    attributes = None
    with codecs.open(infilename, 'r', 'utf-8') as infile:
        reader = csv.reader(infile, quotechar='"', delimiter=',',
                            quoting=csv.QUOTE_ALL)
        attributes = next(reader)
        attributes.append('Hashtag')

        rows = []
        for row in reader:
            hashtag = '(None)'
            for key, value in HASH_TAGS.items():
                if key in row[2]:
                    hashtag = value
                    break
            row.append(hashtag)
            rows.append(row)

    with codecs.open(outfilename, 'w', 'utf-8') as outfile:
        writer = csv.writer(outfile, quotechar='"', delimiter=',',
                            quoting=csv.QUOTE_ALL)
        writer.writerow(attributes)
        writer.writerows(rows)

if __name__ == "__main__":
    in_filename = 'data/3month_b.csv'
    out_filename = 'data/in.csv'
    if len(sys.argv) > 1:
        in_filename = sys.argv[1]
    if len(sys.argv) > 2:
        out_filename = sys.argv[2]
    convert_file(in_filename, out_filename)
    sys.exit(0)
