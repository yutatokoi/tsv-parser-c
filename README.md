# TSV Parser in C

(Code is being released in a few weeks, once grading is finished)

This is my first program written in C. ~600 lines of code.

Given a .tsv file, the program will:
1. Store the data into a 2-D array of strings
2. Sort the data by the columns specified through the command line
3. Generate a tabulated report that shows counts of rows matching the same selected column combination

```
mac: ./soln 4 2 < test0.tsv
Country
    Event    Count
------------------
China
    Cycling      1
    Swimming     1
Indonesia
    Swimming     1
New Zealand
    Cycling      2
    Swimming     1
------------------
```

