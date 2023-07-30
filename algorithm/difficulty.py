#!/usr/bin/python3
# python3 difficulty.py

import sys
import re
import argparse

MAX_N_TASKS = 8
MAX_DIFFICULTY = 9
colors = ["?", "灰", "茶", "緑", "水", "青", "黄", "橙", "赤", "赤<"]

# 1行1コンテストに対応する。
# コンテスト名(もしあれば)三桁のID, ':', A..H問題の結果または難易度
re_contest_pattern = "^([\\D]*\\d{3}):(.{0," + str(MAX_N_TASKS) + "})"
re_id_pattern = "^[\\D]*(\\d{3})"

def collect_results(lower_id, upper_id, difficulty_filename, result_filename):
    difficulty_map = {}
    with open(difficulty_filename) as f:
        for line in f:
            s = line.strip()
            m = re.match(re_contest_pattern, s)
            if not m:
                continue
            key = m.groups()[0]
            value = m.groups()[1]
            difficulties = [0] * MAX_N_TASKS
            for index, d in enumerate(value):
                if d.isdigit():
                    difficulties[index] = int(d)
                difficulty_map[key] = difficulties

    result_table = []
    with open(result_filename) as f:
        for line in f:
            s = line.strip()
            m = re.match(re_contest_pattern, s)
            if not m:
                continue
            key = m.groups()[0]
            value = m.groups()[1]
            result = [" "] * MAX_N_TASKS
            for index, mark in enumerate(value):
                result[index] = mark
            result_table += [(key, result)]

    result_table.sort(key=lambda x: x[0])
    wins = [0] * MAX_DIFFICULTY
    losts = [0] * MAX_DIFFICULTY

    result_lines = ""
    for [key, result] in result_table:
        m = re.match(re_id_pattern, key)
        id = int(m.groups()[0])
        if id < lower_id or id > upper_id:
            continue

        if not key in difficulty_map:
            print("Warning: missing {}".format(key))
            continue

        difficulties = difficulty_map[key]
        line = ""
        for index, difficulty in enumerate(difficulties):
            difficulty_str = str(difficulty)
            mark = result[index]
            result_str = "  "
            if difficulty == 0:
                pass
            elif mark == "+":
                wins[difficulty] = wins[difficulty] + 1
                result_str = str(difficulty) + result[index]
            elif mark != " ":
                losts[difficulty] = losts[difficulty] + 1
                result_str = str(difficulty) + result[index]
            line += " " + result_str
        result_lines += key + ":" + line + "\n"

    n_wins = 0
    n_losts = 0
    summary_lines = "difficulty: wins+losts=total\n"
    for [g, [w, l]] in enumerate(zip(wins, losts)):
        if g < 1 or (w + l) <= 0:
            continue
        s = str(g) + colors[g] + ": " + str(w) + "+" + str(l) + "=" + str(w + l)
        n_wins = n_wins + w
        n_losts = n_losts + l
        summary_lines = summary_lines + s + "\n"

    s = "total: " + str(n_wins) + "+" + str(n_losts) + "=" + str(n_wins + n_losts) + "\n"
    summary_lines = summary_lines + s
    return (result_lines, summary_lines)

def parse_command_line():
    parser = argparse.ArgumentParser()
    ## コンテスト番号の下限
    parser.add_argument("--lower_id", dest="lower_id", type=int,
                        default=0, help="Lowest contest ID")
    ## コンテスト番号の上限
    parser.add_argument("--upper_id", dest="upper_id", type=int,
                        default=1000000, help="Hight contest ID")
    ## 問題の難易度
    ## "297: 112 5" はコンテストID 297の1..5問目の難易度が
    ## 1(灰),1(灰),2(茶),不明,5(青) という意味。6問目以降は問題が無いか難易度が不明。
    parser.add_argument("--difficulty_filename", dest="difficulty_filename", type=str,
                        default="incoming_data/difficulty_auto_abc.csv", help="Textfile for difficulty of tasks")
    ## 問題を解いた結果
    ## "297:  ++ -" はコンテストID 297の1..5問目を
    ## 未回答、解けた、解けた、未回答、解けなかったという意味。6問目以降は未回答。
    parser.add_argument("--result_filename", dest="result_filename", type=str,
                        default="results.txt", help="Textfiles for result of tasks")
    args = parser.parse_args()
    return args

def execute_all():
    args = parse_command_line()
    lower_id = args.lower_id
    upper_id = args.upper_id
    difficulty_filename = args.difficulty_filename
    result_filename = args.result_filename

    result_lines, summary_lines = collect_results(
        lower_id = lower_id, upper_id = upper_id,
        difficulty_filename = difficulty_filename, result_filename = result_filename)
    print(result_lines)
    print(summary_lines)

if __name__ == "__main__":
    execute_all()
