Yaf_Assist
================

``` php
class Yaf_Database_MySQL {
	/**
	 * 初始化数据库对象获取逻辑
	 * @param $masters array 使用如下结构提供主库配置：
	 * 	[["host"=>"127.0.0.1", "port"=>3306, "user"=>"username", "pass"=>"password", "name"=>"database_name"],...]
	 * @param $slaves array 结构与 $master 相同
	 */
	public static function bootstrap(array $masters, array $slaves) {}
	/**
	 * 获取 $index 对应的主库对象实例
	 * @return Yaf_Database_MySQL
	 */
	public static function &getMaster(integer $index = 0): Yaf_Database_MySQL;
	public static function &getSlave(integer $index = 0): Yaf_Database_MySQL;
	/**
	 * 内部保留的 mysqli 对象实例
	 * @var $mysqli mysqli
	 */
	public $mysqli;
	public function __construct(mysqli &$db) {}
	/**
	 * 提供类似 sprintf 形式的 SQL 拼接，并调用 $this->mysqli->query() 实际执行
	 * 注意：参数会自动调用 $this->mysqli->escape() 进行防注入转移
	 * @example
	 * 	$this->formatQuery("INSERT INTO `table` VALUES(?,?)", "ab'c", "def");
	 *  // 等效 $this->query("INSERT INTO `table` VALUES('ab\\'c', 'def')");
	 */
	public function formatQuery(string $format, ...) {};

	// 其余调用转接到 $this->mysqli 
	public function __call($name, $args) {}
	public function __get($name) {}
	public function __isset($name) {}
}
```