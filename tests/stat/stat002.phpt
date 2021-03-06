--TEST--
stat
--INI--
psi.directory={PWD}/../../psi.d:{PWD}
--SKIPIF--
<?php
extension_loaded("psi") or die("skip - need ext/psi");
PHP_OS === "Darwin" or die("skip - only for OSX");
?>
--FILE--
===TEST===
<?php
var_dump(psi\stat(__FILE__, $stat), $stat);
?>
===DONE===
--EXPECTF--
===TEST===
int(0)
array(20) {
  ["st_dev"]=>
  int(%d)
  ["st_ino"]=>
  int(%d)
  ["st_mode"]=>
  int(%d)
  ["st_nlink"]=>
  int(1)
  ["st_uid"]=>
  int(%d)
  ["st_gid"]=>
  int(%d)
  ["st_rdev"]=>
  int(%d)
  ["st_size"]=>
  int(75)
  ["st_atimespec"]=>
  array(2) {
    ["tv_sec"]=>
    int(1%d)
    ["tv_nsec"]=>
    int(%d)
  }
  ["st_atime"]=>
  int(1%d)
  ["st_mtimespec"]=>
  array(2) {
    ["tv_sec"]=>
    int(1%d)
    ["tv_nsec"]=>
    int(%d)
  }
  ["st_mtime"]=>
  int(1%d)
  ["st_ctimespec"]=>
  array(2) {
    ["tv_sec"]=>
    int(1%d)
    ["tv_nsec"]=>
    int(%d)
  }
  ["st_ctime"]=>
  int(1%d)
  ["st_birthtimespec"]=>
  array(2) {
    ["tv_sec"]=>
    int(1%d)
    ["tv_nsec"]=>
    int(%d)
  }
  ["st_birthtime"]=>
  int(1%d)
  ["st_blksize"]=>
  int(%d)
  ["st_blocks"]=>
  int(%d)
  ["st_flags"]=>
  int(%d)
  ["st_gen"]=>
  int(%d)
}
===DONE===
