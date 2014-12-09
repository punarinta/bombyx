<?php

$xy = microtime(true);
$n = 10000;
$arr = [1, 2, 3, 4, 5];

while (--$n)
{
    $a = $arr[2];
}

// multiplication and formatting do not count as they go after the subtraction
echo ("\nExecution took: " . number_format((microtime(true) - $xy) * 1000000, 0, '.', '') . " microsecond(s).\n");
