var appCtrl = angular.module('appCtrl', []);

appCtrl.controller("menuList", function ($scope) {
    $scope.items = Array();
    for (i=0;i<10;i++)
    {
        $scope.items.push({"index": i})
    }

});
