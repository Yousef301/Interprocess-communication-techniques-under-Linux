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
            – Special characters must be encoded.
            – Numbers are encoded as 1,000,000 - number.
            – Each encoded column must have a prefix or suffix added to it so that the receiver
            process is able to identify the column number correctly.
        
      • Helper processes will continuously swap the messages that are present in the shared
      memory to make it hard for spy processes to get all the columns of the file. For
      example, a particular helper process might at some point swap between the encoded
      messages in locations 3 & 10 of the shared memory.

      • Spy processes will continuously access the shared memory locations randomly to get
      the encoded messages before sending them to the master spy process.

      • The master spy process tries to order the columns in the right order after getting
      them from the spy processes. It will drop columns it already received. When the
      master spy is confident it got all the columns, it tries to decode the messages in a
      file called spy.txt before informing the parent process.

      • The receiver process will continuously access the shared memory locations randomly
      to get the encoded messages. Similar to the master spy process, it will order the
      columns it gets in the right order and drops the columns it already received. When
      it is confident it got all the columns, it tries to decode the messages in a file called
      receiver.txt before informing the parent process.

      • The parent process decides if the receiver process was able to get the correct file
      from the sender before the master spy process. If true, then the operation is labeled
      a successful operation. Otherwise, it is labeled as a failed operation.

      • The simulation ends if any of the following is true:
            – The number of failed decoding operations by the receiver exceeds a user-defined
            threshold.
            – The number of successful decoding operations by the receiver exceeds a user-
            defined threshold.

#What you should do:
