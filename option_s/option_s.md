### Option S: Compare two measurement files

Compares two CGATs measurement files. They must have the same number
of patches, in exactly the same order. And must include RGB and LAB fields. 
Other fields are optional. The Delta E 1976's are calcuated and a
summary printed including a histogram in 10% increments.


    i1patches -s file1.txt file2.txt\n"
        Compare two measurement files that have the same RGB values in the same sequence.
        Useful to look at time variation in colors, paper/ink longevity.


Here we compare measurement files taken 3 then 9 minutes after printing to measurements
made after 24 hours.

    Command: i1patches  -s y_24h_M2.txt y_3m_M2.txt
    Average Overall Saturation change: 0.05
    Average Overall Lightness (L*) change: -0.17

                                          dE distribution (percent less than)
                        Ave         10%   20%   30%   40%   50%   60%   70%   80%   90%
      Delta E 1976:    0.43        0.16  0.21  0.26  0.31  0.36  0.42  0.49  0.60  0.80


    Command: i1patches  -s y_24h_M2.txt y_9m_M2.txt
    Average Overall Saturation change: 0.11
    Average Overall Lightness (L*) change: -0.05

                                          dE distribution (percent less than)
                        Ave         10%   20%   30%   40%   50%   60%   70%   80%   90%
      Delta E 1976:    0.23        0.07  0.10  0.13  0.16  0.19  0.22  0.26  0.32  0.45
