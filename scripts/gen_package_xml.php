<?php

function add($dir, $mask, $role, $node, $indent = "    ") {
	$doc = $node->ownerDocument;
	foreach (glob("$dir/$mask", GLOB_BRACE) as $file) {
		$fileNode = $doc->createElement("file");
		$fileNode->setAttribute("role", $role);
		$fileNode->setAttribute("name", basename($file));
		
		$node->appendChild($doc->createTextNode($indent));
		$node->appendChild($fileNode);
		$node->appendChild($doc->createTextNode("\n"));
	}
	foreach (glob("$dir/*", GLOB_ONLYDIR) as $dir) {
		$dirNode = $doc->createElement("dir");
		$dirNode->setAttribute("name", basename($dir));
		$dirNode->appendChild($doc->createTextNode("\n"));
		
		add($dir, $mask, $role, $dirNode,  "$indent ");
		
		$dirNode->appendChild($doc->createTextNode($indent));
		
		$node->appendChild($doc->createTextNode($indent));
		$node->appendChild($dirNode);
		$node->appendChild($doc->createTextNode("\n"));
	}
}

$doc = new DOMDocument;
$doc->load(__DIR__."/../package.xml");

$src = $doc->createElement("dir");
$src->setAttribute("name", "src");
$src->appendChild($doc->createTextNode("\n"));
add(__DIR__."/../src", "*.{c,h,y,re,inc}", "src", $src);
$src->appendChild($doc->createTextNode("   "));

$tests = $doc->createElement("dir");
$tests->setAttribute("name", "tests");
$tests->appendChild($doc->createTextNode("\n"));
add(__DIR__."/../tests", "*.{phpt,inc,psi}", "test", $tests);
$tests->appendChild($doc->createTextNode("   "));

$xpath = new DOMXpath($doc);
$xpath->registerNamespace("p", "http://pear.php.net/dtd/package-2.0");
$root = $xpath->query("/p:package/p:contents/p:dir[@name='/']")->item(0);
$root->replaceChild($src,
		$xpath->query("/p:package/p:contents/p:dir[@name='/']/p:dir[@name='src']")->item(0));
$root->replaceChild($tests,
		$xpath->query("/p:package/p:contents/p:dir[@name='/']/p:dir[@name='tests']")->item(0));

$doc->save(__DIR__."/../package.xml");