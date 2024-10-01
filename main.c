/* Program to generate hierarchical reports from TSV structured data

   Skeleton program written by Alistair Moffat, ammoffat@unimelb.edu.au,
   August 2024, with the intention that it be modified by students
   to add functionality, as required by the assignment specification.
   All included code is (c) Copyright University of Melbourne, 2024
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

#define CHAR_CR '\r' /* CR character, in PC-format text files */
#define CHAR_NL '\n' /* Newline character, ends each input line */
#define CHAR_TB '\t' /* Tab character, ends each TSV field */

#define STATUS_EOL 1 /* Return value for end of line field */
#define STATUS_EOF 2 /* Return value for end of file field */
#define STATUS_NML 3 /* Return value if neither of previous two */

#define MAXFIELDLEN 50   /* Maximum field length */
#define MAXCOLUMNS  30   /* Maximum number of columns per line in input data */
#define MAXLINES    1000 /* Maximum number of input lines in input data */

#define STAGE_1 1
#define STAGE_2 2
#define STAGE_3 3

#define DELIMITER '~'     /* Character used to separate fields in Stage 3 */
#define DELIMITER_STR "~" /* String version of the delimiter */

#define TWO_SPACE_INDENT "  "

#define MAXDIGITS_COLS 2 /* Represents the max number of digits in any column */

/* Minimum number of spaces a field must take up when being printed out in 
Stage 1 and Stage 2 */
#define MIN_WIDTH_OF_FIELD 10 

/* Number of spaces per indent in the output of Stage 3 */
#define SPACES_IN_INDENT 4

/* Used to clear the memory allocated to a field, so as to not access invalid 
characters next time it is used/accessed */
#define EMPTY_FIELD ((const field_t){'\0'})

/* Minimum number of spaces that the number of appearances of an entry takes 
up when printing in Stage 3 */
#define MIN_WIDTH_COUNT 5

/* Additional number of '-' needed when printing out separators */
#define SEPARATOR_CONST 6

/* One tsv field, stored within a fixed-length character string */
typedef char field_t[MAXFIELDLEN + 1];
/* One row of data from the tsv, stored within a fixed-length array of 
field_t */
typedef field_t row_t[MAXCOLUMNS];
/* Data from a tsv file, stored within a fixed-length array of row_t*/
typedef row_t tsv_t[MAXLINES];

/* One row of data from the tsv, where all fields are stored in a 
concatenated array, with a delimiter between each field */
typedef char entry_t[MAXFIELDLEN * MAXCOLUMNS + MAXCOLUMNS];
/* Data from a tsv file, where each row of data is represented as an entry_t */
typedef entry_t entries_t[MAXLINES];

/**************************************************************/

void do_stage1(tsv_t dest, int *rows, int *cols);

void do_stage2(tsv_t input, int argc, int selected_cols[], int row_count, int 
    col_count);
void insertion_sort(tsv_t input, int row_count, int col_count, int keys[], int 
    key_count);
int rowcmp(row_t row1, row_t row2, int keys[], int key_count);
void row_swap(row_t *p1, row_t *p2);

void do_stage3(tsv_t input, int argc, int selected_cols[], int row_count, int 
    col_count);
void print_stage3(entries_t entries, int appearances[], int num_entries, 
    int longest_entry_len);
void print_tabulated_entry(entry_t current_entry, row_t printed);
int entry_len(entry_t current_entry);

void print_row(row_t header, row_t row, int row_num, int cols);
int int_max(int num1, int num2);
void print_new_line(void);
void print_stage_start(int stage);
void print_tadaa(void);
void print_separator(int count);

int mygetchar(void);
int getfield(field_t dest);

/**************************************************************/

/* main program provides traffic control */
int
main(int argc, char *argv[]) {
    tsv_t input;
    int row_count, col_count;
    do_stage1(input, &row_count, &col_count);

    /* Program must exit after Stage 1 if no column numbers are provided on the
    command line */
    if (argc == 1) {
        print_tadaa();
        return 0;
    }

    /* Build an array that holds the column keys, specified by the command-line 
    arguments passed in at execution */
    int selected_cols[--argc];
    for (int i = 0; i < argc; i++) {
        selected_cols[i] = atoi(argv[i + 1]) - 1;
    }

    do_stage2(input, argc, selected_cols, row_count, col_count);
    do_stage3(input, argc, selected_cols, row_count, col_count);

    print_tadaa();
    return 0;
}

/**************************************************************/

/* Stage 1
   -- Read TSV input stream from `stdin`
   -- Build a 2d array of strings from the input
   -- Print row and column count of the input
   -- Print the entries of the last row
   -- Update the pointers given in the argument for further reference within
   the main() function.
*/
void
do_stage1(tsv_t dest, int *rows, int *cols) {
    print_stage_start(STAGE_1);

    int row_count = 0, col_num = 0, field_count = 0, current_status;
    field_t current_field;
    row_t current_row;

    /* Read and store words, one by one into the 2d array of strings */
    while ((current_status = getfield(current_field)) != STATUS_EOF) {
        field_count++;
        strcpy(current_row[col_num++], current_field);

        if (current_status == STATUS_EOL) {
            col_num = 0;
            memcpy(dest[row_count++], current_row, sizeof(row_t));
        }
    }

    /* Calculate the number of colums in the input, and adjust row count to
    account for the first header row */
    int col_count = field_count / row_count;
    row_count--;

    /* Print out required output for Stage 1 */
    printf("input tsv data has %d rows and %d columns\n", row_count, col_count);

    row_t header;
    memcpy(header, dest[0], sizeof(row_t));
    row_t row_last;
    memcpy(row_last, dest[row_count], sizeof(row_t));
    print_row(header, row_last, row_count, col_count);

    print_new_line();

    /* Update and pass back the number of rows and columns read in */
    *rows = ++row_count;
    *cols = col_count;
}

/**************************************************************/

/* Stage 2
   -- Sort the input data, based on the column numbers provided at execution
   -- Print out first, middle, and last row of the sorted data
*/
void
do_stage2(tsv_t input, int argc, int selected_cols[], int row_count, int 
col_count) {
    print_stage_start(STAGE_2);

    row_t header;
    memcpy(header, input[0], sizeof(row_t));
    /* Print formatting as specified by the assignment */
    for (int i = 0; i < argc; i++) {
        if (i == 0) {
            printf("sorting by \"%s\"", header[selected_cols[i]]);
        }
        else {
            printf("%s then by \"%s\"", TWO_SPACE_INDENT, 
            header[selected_cols[i]]);
        }

        if (i != (argc - 1)) {
            printf(",\n");
        }
        else {
            printf("\n");
        }
    }

    /* Sort the data based on the column key(s) provided at execution */
    insertion_sort(input, row_count, col_count, selected_cols, argc);

    /* Print out first, middle, and last data rows */
    int index_first_row = 1;
    row_t row_first;
    memcpy(row_first, input[index_first_row], sizeof(row_t));
    print_row(header, row_first, index_first_row, col_count);

    int index_middle_row = ceil((row_count - 1) / 2.0);
    row_t row_middle;
    memcpy(row_middle, input[index_middle_row], sizeof(row_t));
    print_row(header, row_middle, index_middle_row, col_count);

    int index_last_row = row_count - 1;
    row_t row_last;
    memcpy(row_last, input[index_last_row], sizeof(row_t));
    print_row(header, row_last, index_last_row, col_count);

    print_new_line();
}

/**************************************************************/

/* Sort the given 2d array of strings (input) based on the column number(s)
   given (keys). The sort is stable. Sorts in increasing order.
*/
void
insertion_sort(tsv_t input, int row_count, int col_count, int keys[], int 
key_count) {
    /* Skip the header row to keep it in place */
    for (int i = 1; i < row_count; i++) {
        for (int j = i - 1; j > 0 && (rowcmp(input[j + 1], input[j], keys, 
        key_count) < 0); j--) {
            row_swap(&input[j + 1], &input[j]);
        }
    }
}

/**************************************************************/

/* Compares two rows of data by the key(s) indicating which columns to use in 
   the comparison, and in what order they should be referenced.
*/
int
rowcmp(row_t row1, row_t row2, int keys[], int key_count) {
    for (int i = 0; i < key_count; i++) {
        int cmp = strcmp(row1[keys[i]], row2[keys[i]]);
        if (cmp == 0) {
            /* The strings in this column were the same, continue on to the 
            next column of the two rows */
            continue;
        } else {
            /* Returns positive integer if the first row is greater */
            /* Returns negative integer if the second row is greater */
            return cmp;
        }
    }
    /* The two rows are the same within the columns specified */
    return 0;
}

/**************************************************************/

/* Exchange the values of the two input rows */
void
row_swap(row_t *p1, row_t *p2) {
    row_t temp_row;
    memcpy(temp_row, *p1, sizeof(row_t));
    memcpy(*p1, *p2, sizeof(row_t));
    memcpy(*p2, temp_row, sizeof(row_t));
}

/**************************************************************/

/* Stage 3
   -- Group together entries based on the columns provided as command-line
   arguments at execution
   -- Calculate the number of apperances of entries
   -- Call separate function to print out the result in tabulated format
*/
void
do_stage3(tsv_t input, int argc, int selected_cols[], int row_count, int 
col_count) {
    print_stage_start(STAGE_3);
    int longest_entry_len = 0;

    /* Temporary storage for current and previous entry */
    entry_t current_entry;
    entry_t previous_entry = "";

    /* Store all unique entries, and the number of times they occur */
    entries_t entries;
    int appearances[MAXFIELDLEN];

    int index = 0; /* The index within the entries and appearances arrays */
    int count = 0;
    /* Iterate through each row of sorted data */
    for (int i = 0; i < row_count; i++) {
        /* Store the current entry as a string, ordering fields by the columns 
        passed in as command-line arguments */
        strcpy(current_entry, "");
        for (int j = 0; j < argc; j++) {
            strcat(current_entry, input[i][selected_cols[j]]);
            if (j < argc - 1) {
                /* Add delimiter for parsing later */
                strcat(current_entry, DELIMITER_STR);
            }
        }

        /* Check if this entry matches the previous one */
        if (strcmp(previous_entry, "") != 0 &&
            strcmp(current_entry, previous_entry) != 0) {
            /* Add the previous entry and its count */
            memcpy(entries[index], previous_entry, sizeof(entry_t));
            appearances[index] = count;
            index++;
            count = 0; /* Reset count for next entry */
            longest_entry_len = int_max(longest_entry_len, 
            entry_len(previous_entry));
            /* Current entry needs to only be compared with previous entry 
            as the data is sorted, so any duplicates would be next together */
        }

        /* Update the previous entry and increment count */
        strcpy(previous_entry, current_entry);
        count++;
    }

    /* Add the last entry, if it hasn't been yet */
    if (count > 0) {
        memcpy(entries[index], previous_entry, sizeof(entry_t));
        appearances[index] = count;
        index++;
        longest_entry_len = int_max(longest_entry_len, 
        entry_len(previous_entry));
    }

    print_stage3(entries, appearances, index, longest_entry_len);
}

/**************************************************************/

/* Handles printing and formatting of the Stage 3 output as specified in the 
assignment instructions */
void
print_stage3(entries_t entries, int appearances[], int num_entries, int 
longest_entry_len) {
    print_separator(longest_entry_len + SEPARATOR_CONST);

    /* Array to keep track of what has already been printed in each column.
    For example, with `./soln 1 4 2 < test2`, the field in the 0th index will 
    change from 2016->2020->2024
    */
    row_t printed;

    /* Print out header in tabulated format */
    entry_t header;
    strcpy(header, entries[0]);

    int header_len = entry_len(header);
    print_tabulated_entry(header, printed);
    int extra_spacing = longest_entry_len - header_len;
    printf("%*s %s", extra_spacing, "", "Count");
    print_separator(longest_entry_len + SEPARATOR_CONST);

    /* Print out data entries in tabulated format */
    for (int i = 1; i < num_entries; i++) {
        entry_t current_entry;
        strcpy(current_entry, entries[i]);

        print_tabulated_entry(current_entry, printed);

        int current_entry_len = entry_len(current_entry);
        int extra_spacing = longest_entry_len - current_entry_len;
        printf("%*s %*d", extra_spacing, "", MIN_WIDTH_COUNT, appearances[i]);
    }
    print_separator(longest_entry_len + SEPARATOR_CONST);
    print_new_line();
}

/**************************************************************/

/* Prints out an entry in tabulated format as specified in Stage 3 */
void
print_tabulated_entry(entry_t current_entry, row_t printed) {
    int indents = 0;
    int c = 0;
    char current_char = current_entry[c];

    field_t current_field;

    /* Field must be cleared to avoid acceessing memory from previous runs */
    memcpy(current_field, EMPTY_FIELD, sizeof(current_field));

    int field_i = 0;

    /* Print every field until we reach the end of the entry */
    while (current_char != '\0') {
        /* Process the current field as we have reached a delimiter */
        if (current_char == DELIMITER) {
            /* This field is a duplicate, so don't print it out */
            if (strcmp(current_field, printed[indents]) == 0) {
                indents++;
                current_char = current_entry[++c];
                field_i = 0;
                memcpy(current_field, EMPTY_FIELD, sizeof(current_field));
                continue;
            /* Field was not duplicate */
            } else {
                print_new_line();

                printf("%-*s", SPACES_IN_INDENT * indents, "");
                printf("%s", current_field);

                /* Store value in printed[] to avoid printing again, if 
                duplicate */     
                memcpy(printed[indents], current_field, sizeof(field_t));

                memcpy(current_field, EMPTY_FIELD, sizeof(current_field));
                field_i = 0;
                indents++;
            } 
        } else {
            current_field[field_i] = current_char;
            field_i++;
        }
        current_char = current_entry[++c];
    }
    print_new_line();
    printf("%-*s", SPACES_IN_INDENT * indents, "");
    printf("%s", current_field);

    /* Store value in printed[] to avoid printing again, if duplicate */
    memcpy(printed[indents], current_field, sizeof(field_t));
    
    memcpy(current_field, EMPTY_FIELD, sizeof(current_field));
}

/**************************************************************/

/* Returns the length of an entry if it were to be printed out in tabulated 
   format as specified in Stage 3
*/
int
entry_len(entry_t current_entry) {
    int max_field_len = 0;
    int indents = 0;
    int c = 0;
    char current_char = current_entry[c];

    while (current_char != '\0') {
        /* Calculate length of the current field */
        int current_field_len = 0;
        while ((current_char != '\0') && (current_char != DELIMITER)) {
            current_field_len++;
            current_char = current_entry[++c];
        }

        /* Adjust for the indentation of this field */
        current_field_len += (indents * SPACES_IN_INDENT);

        max_field_len = int_max(max_field_len, current_field_len);

        /* Move on to next character if the current character is a delimiter */
        if (current_char == DELIMITER) {
            current_char = current_entry[++c];
        }
        indents++;
    }
    return max_field_len;
}

/**************************************************************/

/* Other helper functions */

/* Prints the information in the row of data as specified by the assignment
   instructions
*/
void
print_row(row_t header, row_t row, int row_num, int cols) {
    printf("row %d is:\n", row_num);
    for (int i = 0; i < cols; i++) {
        int col_number = i + 1;
        printf("%s%*d: %-*s %s\n", TWO_SPACE_INDENT, MAXDIGITS_COLS, 
        col_number, MIN_WIDTH_OF_FIELD,header[i], row[i]);
    }
}

/* Given two integers, returns the larger one */
int 
int_max(int num1, int num2) {
    if (num1 >= num2) {
        return num1;
    } else {
        return num2;
    }
}

void
print_new_line(void) {
    printf("\n");
}

/* Outputs a line of text to indicate the start of a new stage */
void
print_stage_start(int stage) {
    /* No new line at the end in order to account for how the rest of Stage 3 
    is implemented */
    if (stage == STAGE_3) {
        printf("Stage %d", stage);
    } else {
        printf("Stage %d\n", stage);
    }
}

/* Outputs a line of text to indicate the program ran to the end */
void
print_tadaa(void) {
    printf("ta daa!\n");
}

/* Outputs a line of text which is a specified number of '-' */
void
print_separator(int count) {
    print_new_line();
    while (count > 0) {
        printf("-");
        count--;
    }
}

/**************************************************************/

/* Read characters and build a string, stopping when a tab or newline
   as encountered, with the return value indicating what that
   terminating character was
*/
int
getfield(field_t dest) {

	int ch, nchs=0;
	int status=STATUS_NML;

	dest[0] = '\0';
	while ((ch=mygetchar())
		&& (ch != CHAR_TB) && (ch != CHAR_NL) && (ch != EOF)) {

		if (nchs < MAXFIELDLEN) {
			/* Ok to save this character */
			dest[nchs++] = ch;
			dest[nchs] = '\0';
		} else {
			/* Silently discard extra characters if present */
		}
	}

	/* Return status is defined by what character stopped the loop */
	if (ch == EOF) {
		status = STATUS_EOF;
	} else if (ch == CHAR_NL) {
		status = STATUS_EOL;
	} else if (ch == CHAR_TB) {
		status = STATUS_NML;
	}
	return status;
}

/**************************************************************/

/* Read a single character, bypassing any CR characters encountered,
   so as to work correctly with either PC-type or Unix-type input
 */
int
mygetchar(void) {
	int c;
	while ((c=getchar()) == CHAR_CR) {
		/* Empty loop body */
	}
	return c;
}
