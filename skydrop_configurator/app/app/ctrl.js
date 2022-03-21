"use strict";

app.directive('ctrlNumber', function() {
    return {
        restrict: 'E',
        scope: {
            max: "=",
            min: "=",
            step: "=",
            ngModel: '='
        },
        template: function(element, attrs) {
            var html = '<input ng-model="ngModel" class="form-control" type="number" min={{min}} max={{max}} step={{step}} required />';
            return html;
        }
    };
});

app.directive('ctrlSlider', function() {
    return {
        restrict: 'E',
        scope: {
            max: "=",
            min: "=",
            step: "=",
            ngModel: '='
        },
        template: function(element, attrs) {
            var html = '<slider ng-model="ngModel" min=min max=max step=step/>';
            return html;
        }
    };
});

app.directive('ctrlBool', function() {
    return {
        restrict: 'E',
        scope: {
            ngModel: '='
        },
        template: function(element, attrs) {
            var html = '<switch ng-model="ngModel" class="blue" on="Enabled" off="Disabled"></switch>';
            return html;
        }
    };
});

app.directive('ctrlSelect', function() {
    return {
        restrict: 'E',
        scope: {
            ngModel: '=',
            options: '='
        },
        template: function(element, attrs) {
            var html = '<select ng-model="ngModel.option" ng-options="item[0] as item[1] for item in options">';
            html += '</select>';    
            return html;
        }
    };
});

app.directive('ctrlFlags', function() {
    return {
        restrict: 'E',
        scope: {
            ngModel: '=',
            flags: '='
        },
        template: function(element, attrs) {
            var html = '<div ng-repeat="item in flags" class="checkbox">';
            html += '<label>';
            html += '<input type="checkbox" ng-checked="checked(item[0])" ng-click="click(item[0])"/>';
            html += '{{item[1]}}';    
            html += '</div>';    
            return html;
        },
        link: function(scope, element, attrs)
        {
            scope.checked = function(flag)
            {
                return scope.ngModel.flags[flag];
            };
            
            scope.click = function(flag)
            {
            	scope.ngModel.flags[flag] = !scope.ngModel.flags[flag];
            };
        }
    };
});

app.directive('ctrlAltimeter', ['memory', function(memory, $parse, $rootScope) {
    return {
        restrict: 'E',
        scope: {
            ngModel: '=',
            flags: '=',
            options: '=',
            altimeters: '='
        },
        template: function(element, attrs) {
            var html = '<div>';
            html += '<b>Mode</b><br>';
            html += '<ctrl-select ng-model="ngModel.mode" options="options"></ctrl-select><br>';
            html += '<span ng-show="relative"><b>Relative to</b><br>';
            html += '<ctrl-select ng-model="ngModel.altimeter" options="altimeters"></ctrl-select><br></span>';
            html += '<b>Flags</b><br>';
            html += '<ctrl-flags ng-model="ngModel" flags="flags"></ctrl-flags>';
            html += '</div>';
            return html;
        },
        link: function(scope, element, attrs)
        {
        	scope.$watch('ngModel.mode.option', function(value)
        	{
	            scope.relative = "ALT_DIFF" == scope.ngModel.mode.option;
        	});
          
        }
    };
}]);


app.directive('ctrlDoubleselect', ['memory', function(memory, $parse, $rootScope) {
    return {
        restrict: 'E',
        scope: {
            ngModel: '=',
            label1: '=',
            options1: '=',
            mask1: '=',
            label2: '=',
            options2: '=',
            mask2: '=',
        },
        template: function(element, attrs) {
            var html = '<div>';
            html += '<b>{{label1}}</b><br>';
            html += '<ctrl-select ng-model="ngModel.select1" options="options1"></ctrl-select><br>';
            html += '<b>{{label2}}</b><br>';
            html += '<ctrl-select ng-model="ngModel.select2" options="options2"></ctrl-select><br>';
            html += '</div>';
            return html;
        },
    };
}]);

app.directive('ctrlTripleselect', ['memory', function(memory, $parse, $rootScope) {
    return {
        restrict: 'E',
        scope: {
            ngModel: '=',
            label1: '=',
            options1: '=',
            mask1: '=',
            label2: '=',
            options2: '=',
            mask2: '=',
            label3: '=',
            options3: '=',
            mask3: '=',
        },
        template: function(element, attrs) {
            var html = '<div>';
            html += '<b>{{label1}}</b><br>';
            html += '<ctrl-select ng-model="ngModel.select1" options="options1"></ctrl-select><br>';
            html += '<b>{{label2}}</b><br>';
            html += '<ctrl-select ng-model="ngModel.select2" options="options2"></ctrl-select><br>';
            html += '<b>{{label3}}</b><br>';
            html += '<ctrl-select ng-model="ngModel.select3" options="options3"></ctrl-select><br>';
            html += '</div>';
            return html;
        },
    };
}]);

app.directive('ctrlDampening', function() {
    return {
        restrict: 'E',
        scope: {
            max: "=",
            min: "=",
            step: "=",
            ngModel: '='
        },
        template: function(element, attrs) {
            var html = '<input ng-model="num" class="form-control" type="number" min={{min}} max={{max}} step={{step}} required />';
            return html;
        },
        link: function(scope, element, attrs)
        {
            function mul_to_sec(mul)
            {
	            if (mul == 0)
		            return 1;
	            else
		            return Math.round(100.0 / (mul * 100.0)) / 100;
            }

            function sec_to_mul(sec)
            {
	            if (sec == 0.0)
		            return 1;
	            else
		            return (1.0 / (sec * 100.0));
            }        

            var disable_update = false;
            
            scope.$watch('ngModel', function(value)
          	{
            	if (disable_update)
            		disable_update = false;
            	else
        		{
            		disable_update = true;
            		scope.num = mul_to_sec(scope.ngModel);
        		}
          	});
            
            scope.$watch('num', function(val)
            {
            	if (disable_update)
            		disable_update = false;
            	else
            	{
            		disable_update = true;
            		scope.ngModel = sec_to_mul(val);
            	}
            });
        }
    };
});

app.directive('ctrl', function() {
    return {
        restrict: 'E',
        scope: {
            item: '='
        },
        templateUrl:"pages/ctrl.html"
    };
});

app.directive('ctrlTabs', function() {
    return {
        restrict: 'E',
        transclude: true,
        scope: {
            single: '='
        },            
        template: function(){return '<uib-accordion close-others="{{single}}"><div ng-transclude></div></uib-accordion>'; }
    };
});

app.directive('ctrlPanel', function() {
    return {
        restrict: 'E',
        transclude: true,
        scope: {
            name: '=',
        },        
        template: function(element, attrs) {
            var html = '<div class="panel panel-default">';
            html += '<div class="panel-heading"><h4 class="panel-title">';
            html += '{{name}}';
            html += '</h4></div>';
            html += '<div class="panel-body" ng-transclude></div>';
            html += '</div>';
            return html;
        },
        link: function(scope, element, attrs)
        {
        	scope.name = attrs.name.replace(/_/g, " ");
        }	
    };
});


app.directive('ctrlTab', function() {
    return {
        restrict: 'E',
        transclude: true,
        scope: {
            name: '=',
            open: '='
        },        
        template: function(element, attrs) {
            var html = '<uib-accordion-group is-open="open">';
            html += '<uib-accordion-heading>';
            html += '{{name}} <i class="pull-right glyphicon" ng-class="{\'glyphicon-chevron-down\': open, \'glyphicon-chevron-right\': !open}"></i>';
            html += '</uib-accordion-heading>';
            html += '<div ng-transclude></div>';
            html += '</uib-accordion-group>';
            return html;
        },
        link: function(scope, element, attrs)
        {
        	scope.name = attrs.name.replace(/_/g, " ");
        }	
    };
});

app.directive('ctrlText', function() {
    return {
        restrict: 'E',
        scope: {
            max: "=",
            ngModel: '='
        },
        template: function(element, attrs) {
            var html = '<input ng-model="ngModel" class="form-control" type="text" />';
            return html;
        },
        link: function(scope, element, attrs)
        {
        	scope.text = scope.ngModel;
        	
            scope.$watch('ngModel', function(val)
            {
            	if (typeof val === 'undefined')
            		val = "";
            			
            	//Check max len (need to count \0 too!)
            	if (val.length <= scope.max)
            		val = val.substring(0, scope.max - 1);
            	
            	//Check if characters are printable on lcd (32-125)
            	var new_text = "";
            	for (var i in val)
            	{
            		var ord = val.charCodeAt(i);
            		if (ord >= 32 && ord <= 125)
            			new_text += val[i];
            	}

            	scope.ngModel = new_text;
            });
        }	
    };
});

