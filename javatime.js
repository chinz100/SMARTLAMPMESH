window.onload = function () {
    var ddl = document.getElementById('Hourdropdown').getElementsByTagName("select")[0];
    
    for (var i = 0; i <= 23; i++) {
        var theOption = new Option;
        theOption.text = i+" HH";
        theOption.value = i;
        ddl.options[i] = theOption;
      }
      
    var dd2 = document.getElementById('Mindropdown')
      
      for (var i = 0; i <= 59; i++) {
          var theOption = new Option;
          theOption.text = i +" MM";
          theOption.value = i;
          dd2.options[i] = theOption;
      }
    }
    

