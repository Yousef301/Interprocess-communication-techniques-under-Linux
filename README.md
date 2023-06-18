# Interprocess-Communication-Techniques-Under-Linux
We would like to help militant groups into sending encoded messages that only the right
receiver is able to decode using a multi-processing approach. We’ll assume that if the
receiver is able to get the encoded messages before a master spy process and is able to
decode them correctly, then the operation is successful. Otherwise, if a master spy process
is able to get to the messages and decode them first, then the operation has failed.
The application scenario can be explained as follows:

      • A parent process must create a single sender process, single receiver process, a single
      master spy process, a user-defined number of helper processes and a user-defined
      number of spy processes.
      
      • A sender process gets the message from an input file (e.g. sender.txt). The message
      might be composed of multiple lines and multiple paragraphs. The sender should
      split the input file by column based on the blank character between words.

      • The sender process should create as many children processes as needed depending
      on the number of columns in the file. To each child process, a column message must
      be sent containing all the words in that column. The columns should be of equal
      size. As such, if a column contains an empty string in any line, it should be replaced
      by the string “alright”.

      • The children processes should each encode the column message that is passed to
      it before placing the encoded message in the shared memory. Each encoding child
      process acts as follows:
        – For the first column, for each word in the string, we add 1 to the first character
        modulo 26, 2 to the second character modulo 26, etc. The child process re-
        sponsible for encoding that string will place it in the first location in the shared
        memory.
        – For the second column, for each word in the string, we add 2 to the first
        character modulo 26, 4 to the second character modulo 26, etc. The child
        process responsible for encoding that string will place it in the second location
        in the shared memory.
        – Same logic applies to the consecutive columns and words of each column.
        – Special characters must be encoded as follows:
        ! ! 1 ? ! 2 , ! 3
        ; ! 4 : ! 5 % ! 6

        – Numbers are encoded as 1,000,000 - number.
        – Each encoded column must have a prefix or suffix added to it so that the receiver
        process is able to identify the column number correctly.
