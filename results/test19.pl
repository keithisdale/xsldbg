
#! perl -w

#use the package xsldbgmatch
require "xsldbgmatch.pl";

      my $template ="",  $fileName ="", $identifier ="", $state ="", $line ="";
      my $testCount = 0, $failedTestCount = 0, $optionalTestCount = 0;

      my $result = 1, $testName ="test19";
      printf "Running test $testName\n";
        


    $testCount = $testCount + 1;
    
    if (xsldbgmatch::textMatch(
             "Reached template: \"/\"", 
               $testName) == 0 ){
       $failedTestCount = $failedTestCount + 1;
       $result = 0;
    }
  


    $testCount = $testCount + 1;
    
    if (xsldbgmatch::breakpointListMatch(
             "",  
               "" , 
                 "ftp://xsldbg:xsldbg\@127.0.0.1/xsldbg/tests/test19.xsl",
                   "16",
                     $testName) == 0){
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

  
