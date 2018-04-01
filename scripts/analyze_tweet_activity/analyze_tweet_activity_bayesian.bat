echo off
chcp 65001
set PATH=C:\Rtools\bin;C:\Rtools\mingw_64\bin;C:\Program Files\R\R-3.4.4\bin;%PATH%
set TWEET_CSV1=data/in.csv
set TWEET_CSV2=data/in2.csv

python analyze_tweet_activity_bayesian.py --hour_span 2 %TWEET_CSV1% out/converted1.csv
Rscript analyze_tweet_activity_bayesian2.R out/converted1.csv out/chart2_1
python analyze_tweet_activity_bayesian.py --hour_span 4 %TWEET_CSV2% out/converted2.csv
Rscript analyze_tweet_activity_bayesian2.R out/converted2.csv out/chart2_2
Rscript analyze_tweet_activity_bayesian.R out/converted2.csv out/chart2
