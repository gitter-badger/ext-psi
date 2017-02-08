--TEST--
arpa/inet
--INI--
psi.directory={PWD}/../../psi.d:{PWD}
--SKIPIF--
<?php
extension_loaded("psi") or die("skip - need ext/psi");
function_exists("psi\\ntohs") or die("skip - need ntohs()");
?>
--FILE--
===TEST===
<?php
var_dump(psi\htons(psi\ntohs(55)));
var_dump(psi\htonl(psi\ntohl(555555)));
var_dump(psi\ntohs(psi\htons(14080)));
var_dump(psi\ntohl(psi\htonl(595200000)));
?>
===DONE===
--EXPECT--
===TEST===
int(55)
int(555555)
int(14080)
int(595200000)
===DONE===
