<?php

$t = microtime(true);
$n = 10000;

while (--$n)
{
    $a = "hello, " . $n . " world";
}

// multiplication and formatting do not count as they go after the subtraction
echo ("\nExecution took: " . number_format((microtime(true) - $t) * 1000000, 0, '.', '') . " microsecond(s).\n");
