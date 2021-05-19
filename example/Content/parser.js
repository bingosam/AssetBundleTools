$(function(){
	//折叠或者展开详情
	foldTable("packageName");
	foldTable("dependPackage");
	foldTable("redundancies");
	
	//跳转到顶部
	$(".to-top").click(function(){
		$('html, body').animate({scrollTop:0}, 'fast');
	});

///折叠或展开表格///
///id:需要折叠或者展开表格的DIV对应的ID///
function foldTable(id){

$("#"+id+" h3").click(function(){
	$("#"+id+" .table-con").toggle();
	var obj=$("#"+id+" h3 i");
	var value=obj.html();
	if(value=="+"){
		obj.html("-");

	}
	else{
		obj.html("+");
	}
});
}


});


