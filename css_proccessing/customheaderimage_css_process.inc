<?php

  /**
   * Functions to have
   * ----------------------------
   * read css file
   * check if rule exists for path
   * add to css file taking in path
   * edit css file with specifc path
   */
  function customheaderimage_add_rule($page_path, $image) {
    $path = drupal_get_path('module', 'customheaderimage').'css/cssheaderimage_style.css';
    $file = fopen($path, 'w+');
    //$size = filesize($path);

    if(stristr($path, 'node')) {
      //Set up page css styles
      //page-node-2
      $nid = customheaderimage_get_nid_from_path($page_path);
      $class = '.page-node-'.$nid.' #header';
      $style = "{ background-image: url('$image'); }";
      if(fwrite($path, $class . $style)) {
        drupal_set_message('Added CSS Style');
      }
      else {
        drupal_set_message('Error adding CSS style', 'error');
      }
    }
    else {
      //Set up view css styles
      //page-path-to-view
    }
  }

