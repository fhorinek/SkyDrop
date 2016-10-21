"use strict";

app.directive('widget', function() {
    return {
        restrict: 'E',
        scope: {
        	ngModel: '=',
        	key: '=',
        	drag: '=',
        	dstyle: '=',
        },
        templateUrl:"pages/widgets.html"
    };
});

app.directive('widgetSlot', function() {
    return {
        restrict: 'E',
        scope: {
        	ngModel: '=',
        },
        transclude: true,
        template: function(element, attrs) {
            var html = '<div class="gui_widget" ng-style="ngModel.style" ng-drop="true" ng-drop-success="ngModel.drop($data, ngModel)"></div>';
            
            return html;
        },
        link: function(scope, element, attrs)
        {
             scope.$watch('ngModel.content', function(val) {
            	element.children().html(val);
            });
        },        
    };
});

app.controller("screens", ["memory", "$scope", "$q", '$window', "$sce", "$compile", "$timeout", function (memory, $scope, $q, $window, $sce, $compile, $timeout) {
	
	var deferred = $q.defer();

	var disp_w = 84;
	var disp_h = 48;
	var zoom = 1.0;
	var active_layout = false;
	
	$scope.slots = [];
	$scope.list = {};
	$scope.widget_size = {};
	$scope.avalible = true;
	
	$scope.active_page_number = 2;
	$scope.active_page_base = "cfg_gui_pages_" + $scope.active_page_number;
	$scope.active_page_type = $scope.active_page_base + "_type";
	
	deferred.promise.then(undefined, undefined, function (data){
		$scope.list = data;
		$scope.avalible = (memory.layouts) ? true : false;
		
		if (!$scope.avalible)
			return;
		
		$scope.widgets = memory.get_widgets();
		$scope.generate_slots();
	
		$scope.$watch(function () {return $window.innerWidth;}, function (value) {
			$scope.resize();
		}, true);
	
		angular.element($window).bind('resize', function(){
			$scope.$apply();
		});		
		
		$scope.change_page($scope.active_page_number);

		$scope.$watch('list[active_page_type].value.option', function(){
			$scope.refresh();
		});
	
	});	
	
	$scope.$on('$destroy', function() {
		memory.remove_notify(deferred);
	});
	
	var resize_delayed = false;
	
	$scope.resize = function()
	{
		var screen_w = document.getElementById("gui_screen").clientWidth - 4;
		if (screen_w < 0)
		{
			if (resize_delayed == false)
			{
				$timeout($scope.resize, 100);
				console.log("delaying widgets resize!");
			}
			
			resize_delayed = true;
			return;
		}
		resize_delayed = false;
		
		zoom = screen_w / disp_w;
		$scope.screen_style = {height: (disp_h * zoom) + "px"};
		$scope.widget_size.label = {"font-size": (zoom * 4) + "px"};
		$scope.widget_size.value = {"font-size": (zoom * 7) + "px"};
		$scope.widget_size.text = {"font-size": (zoom * 6) + "px"};
		
		for (var i = 0; i < active_layout.number_of_widgets; i++)
			set_widget(i , active_layout.widgets[i]);	
	};
	
	$scope.load_page = function(page)
	{
		$scope.active_page_number = page;
		
		$scope.active_page_base = "cfg_gui_pages_" + $scope.active_page_number;
		$scope.active_page_type = $scope.active_page_base + "_type";
	};
	
	$scope.refresh = function()
	{
		var layout = $scope.list[$scope.active_page_type].value.option;
		active_layout = memory.layouts[layout];
		
		for (var i = 0; i < 9; i++)
			clear_widget(i);
		
		$scope.resize();
	};
	
	$scope.generate_slots = function()
	{
		for (var i=0; i < 9; i++)
		{
			var slot = {};
			
			slot.style = {};
			slot.ident = i;
			slot.content = "";
			slot.drop = $scope.drop;
			
			$scope.slots[i] = slot;
		}		
	};
	
	$scope.load_widgets = function()
	{
		for (var i=0; i < 9; i++)
		{
			var w_key = $scope.list[$scope.active_page_base + "_widgets_" + i].value.option;
			
			$scope.slots[i].key = w_key;
			$scope.slots[i].content = $scope.return_widget(w_key);
		}	
	};

	function clear_widget(n)
	{
		$scope.slots[n].style = {};
	}
	
	function set_widget(n, pos)
	{
		var style = $scope.slots[n].style;
		
		style.left = (pos[0] * zoom) - 2 + "px";
		style.top = (pos[1] * zoom) - 2 + "px";
		style.width = pos[2] * zoom + "px";
		style.height = pos[3] * zoom + "px";
		style.display = "block";
	}
	
	$scope.return_widget = function(w_key)
	{
		var tpl = "<widget dstyle=\"widget_size\" ng-model=\"widgets[\'"+w_key+"\']\"></widget>";
		var cmp = $compile(tpl)($scope);
		
//		console.log("creating widget", w_key, cmp);
		
		return cmp;
	};
	
	$scope.drop = function(w_key, slot)
	{
		slot.content = $scope.return_widget(w_key);
		$scope.list[$scope.active_page_base + "_widgets_" + slot.ident].value.option = w_key;
	};
	
	$scope.change_page = function(page)
	{
//		console.log("loading_page", page);
		
		$scope.load_page(page);
		$scope.refresh();
		$scope.load_widgets();
	};
	
	//init
	memory.getAllValues(deferred);
}]);

