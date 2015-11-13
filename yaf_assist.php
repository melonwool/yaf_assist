<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('yaf_assist')) {
	dl('php_yaf_assist.' . PHP_SHLIB_SUFFIX);
}
$module = 'yaf_assist';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
echo "$br\n";
$function = 'confirm_' . $module . '_compiled';
if (extension_loaded($module)) {
	$str = $function($module);
} else {
	$str = "Module $module is not compiled into PHP";
}
function refcount(&$var) {
    ob_start();
    debug_zval_dump(array(&$var));
    $output = substr(ob_get_clean(), 24);
    $output = preg_replace("/^.+?refcount\((\d+)\).+$/ms", '$1', $output, 1);
    return $output;
}
echo "$str\n";
yaf_database_mysql::bootstrap([
	[
		"host"=>"182.92.228.217",
		"port"=>3306,
		"user"=>"wuhao",
		"pass"=>"CrD5ivfNUBiFuLzf",
		"name"=>"fangxin8_core",
	]
]);
$db = Yaf_Database_Mysql::getMaster();
$ob = DateTime::createFromFormat("Y", "2015");
var_dump("refcount", refcount($db), refcount($ob));
// $rs = $db->formatQuery("SELECT * FROM `pupil` WHERE `pupil_name`=?", $name = "张元棋");
// var_dump("result", $rs);
// var_dump("data", $rs->fetch_all(MYSQLI_ASSOC));

fgetc(STDIN);