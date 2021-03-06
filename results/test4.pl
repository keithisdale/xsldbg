
#! perl -w

#use the package xsldbgmatch
require "xsldbgmatch.pl";

      my $template ="",  $fileName ="", $identifier ="", $state ="", $line ="";
      my $testCount = 0, $failedTestCount = 0, $optionalTestCount = 0;

      my $result = 1, $testName ="test4";
      printf "Running test $testName\n";
        






    $testCount = $testCount + 1;
    
    if (xsldbgmatch::textMatch(
             "Error: Invalid arguments for the command addparam", 
               $testName) == 0 ){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  


    $testCount = $testCount + 1;
    
    if (xsldbgmatch::textMatch(
             "Error: Invalid arguments for the command addparam", 
               $testName) == 0 ){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  


    $testCount = $testCount + 1;
    
    if (xsldbgmatch::textMatch(
             "Error: Unable to find parameter a24", 
               $testName) == 0 ){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  


    $testCount = $testCount + 1;
    
    if (xsldbgmatch::textMatch(
             "Error: Unable to find parameter 10", 
               $testName) == 0 ){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  





    $testCount = $testCount + 1;
    
    if (xsldbgmatch::textMatch(
             "Restart to apply new parameter value", 
               $testName) == 0 ){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  



    $testCount = $testCount + 1;
    
    if (xsldbgmatch::textMatch(
             "Restart to apply new parameter value", 
               $testName) == 0 ){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  


    $testCount = $testCount + 1;
    
    if (xsldbgmatch::textMatch(
             "Restart to apply new parameter value", 
               $testName) == 0 ){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  


    $testCount = $testCount + 1;
    
    if (xsldbgmatch::textMatch(
             "Restart to apply new parameter value", 
               $testName) == 0 ){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  




    $testCount = $testCount + 1;
    
    if (xsldbgmatch::parameterMatch(
             "0",  
               "help" , 
                 "'step'",
                   $testName) == 0){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  
    $testCount = $testCount + 1;
    
    if (xsldbgmatch::parameterMatch(
             "1",  
               "quotedparam" , 
                 "step or where",
                   $testName) == 0){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  
    $testCount = $testCount + 1;
    
    if (xsldbgmatch::parameterMatch(
             "2",  
               "results" , 
                 "result/data",
                   $testName) == 0){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  
    $testCount = $testCount + 1;
    
    if (xsldbgmatch::parameterMatch(
             "3",  
               "version" , 
                 "'1.0'",
                   $testName) == 0){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  




    $testCount = $testCount + 1;
    
    if (xsldbgmatch::textMatch(
             "Restart to apply removed parameter value", 
               $testName) == 0 ){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  

    $testCount = $testCount + 1;
    
    if (xsldbgmatch::textMatch(
             "Restart to apply removed parameter value", 
               $testName) == 0 ){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  




    $testCount = $testCount + 1;
    
    if (xsldbgmatch::parameterMatch(
             "1",  
               "quotedparam" , 
                 "step or where",
                   $testName) == 0){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  
    $testCount = $testCount + 1;
    
    if (xsldbgmatch::parameterMatch(
             "2",  
               "results" , 
                 "result/data",
                   $testName) == 0){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  




    $testCount = $testCount + 1;
    
    if (xsldbgmatch::textMatch(
             "No parameters present", 
               $testName) == 0 ){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  




    if ($result == 1){
      if ($failedTestCount != 0){
       printf "Success but some optional tests failed : $failedTestCount of $testCount tests\n";
      }
      printf "\n\n";
      exit(0);
    }else{
      printf "Failure : $failedTestCount of $testCount tests failed\n";
      printf "\n\n";
      exit(1);
    }

  