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
