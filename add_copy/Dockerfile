FROM ubuntu:focal

# docker build . -f Dockerfile -t add_copy && docker run add_copy
COPY echo.sh /root/echo.sh

# cp first1/**/* dest/ (with trailing /)
# The destination directory is created if it does not exist
COPY first1 /root/first1/

# cp first2/**/* dest (without trailing /)
COPY first2 /root/first2

# ADD instead of COPY
ADD first1 /root/sub3/first3/
ADD first2 /root/sub4/first4

# All files in subdir/**/* are copied at /root/dest/ !
# https://stackoverflow.com/questions/30215830/dockerfile-copy-keep-subdirectory-structure
COPY first1/* /root/flat1/

RUN mkdir -p /root/flat2
# /root/dest without trailing / must exist
COPY first2/* /root/flat2

CMD bash /root/echo.sh
