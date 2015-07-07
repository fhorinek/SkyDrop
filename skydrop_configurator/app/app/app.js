

var appCtrl = angular.module('appCtrl', []);



appCtrl.controller("pageView", ['$scope', '$http', function ($scope, $http) {



}]);


appCtrl.controller("audioProfile", ['$scope', '$http', function ($scope, $http) {
    $scope.items = [
        {"a":1},
        {"a":2},
        {"a":3},
    
    ];


}]);


var app = angular.module('app', [
    'ngRoute',
    'appCtrl']);

app.config(['$routeProvider',
  function($routeProvider) {
    $routeProvider.
      when('/test', {
        templateUrl: 'pages/test.html',
        controller: 'pageView'
      }).
      when('/audio_profile', {
        templateUrl: 'pages/audio_profile.html',
        controller: 'pageView'
      }).      
      otherwise({
        redirectTo: '/test'
      });
  }]);
  
  app.controller("menuList", ['$scope', '$http', function ($scope, $http) {
    $scope.groups = 
    [
        [
            {"title": "Audio Profile editor", "ref": "audio_profile"},
            {"title": "Test", "ref": "test"}
        ],
        [
            {"title": "Test", "ref": "test"},
            {"title": "Test", "ref": "test"}
        ]
    ];
}]);
