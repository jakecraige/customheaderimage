<?php
// $Id$

/**
 * @file
 * Configuration for Custom Header Image Module
 */

/**
 * Callback for default page and displays  the path specific form to
 * set up images for those pages
 * @return [form render array]
 */
function customheaderimage_display_pathspecific() {
  $all_paths_array = customheaderimage_get_paths();
  return customheaderimage_create_table($all_paths_array);
}

/**
 * Callback for Global Tab - Will Display form for global settings
 * @return [array] [form]
 */
function customheaderimage_display_global() {
  return  drupal_get_form('customheaderimage_image_form');
}

/**
 * [customheaderimage_get_paths description]
 * @return [type] [description]
 */
function customheaderimage_get_paths() {
  $page_paths = customheaderimage_get_page_paths();
  $view_paths = customheaderimage_get_view_paths();

  //Holds Associative Key=Title Value=Path Array
  return customheaderimage_create_form_array($page_paths, $view_paths);
}

/**
 * Creates assoc array with title as key and path as value
 * of all regular pages and views
 * @param  [array] $paths [page paths no title]
 * @param  [array] $views [view paths no title]
 * @return [array]        [title=>path assoc array]
 */
function customheaderimage_create_form_array($paths, $views) {
  $titles = array();
  foreach($paths as $key=>$value) {
    $titles[customheaderimage_title_from_nid($key)] = $value;
  }
  foreach($views as $view_path) {
    $name = customheaderimage_name_from_view_path($view_path);
    $key = customheaderimage_title_from_view_name($name);

    $titles[$key] = $view_path;
  }
  return $titles;
}
/**
 * Returns HTML of table to display paths and titles on config page
 * @param  [array] $paths [assoc array of title=>path]
 * @return [string]        [table]
 */
function customheaderimage_create_table($paths) {
  $header = array(
    'title' => t('Title'),
    'path'     => t('Path'),
    'image'    => t('Image'),
    'operations'    => t('Operations'),
  );
  $rows = array();
  $count = 0;
  foreach ($paths as $title=>$path) {
    $base_path = variable_get('file_public_path', conf_path() . '/files');
    $image = customheaderimage_image_name($path);
    $rows[] = array(
                'title' => l($title, $path),
                'path' => $path,
                'image' => l($image, $base_path.'/'.$image),
                'operations'
                    => l('Edit', 'admin/config/customheaderimage/edit/'
                                  .customheaderimage_slashtodash($path)),
    );
    if($image) {
      $rows[$count]['operations'] .= ' | ' . l('Delete', 'admin/config/customheaderimage/delete/'
                             .customheaderimage_slashtodash($path));
    }
    $count++;
  }

  return theme('table', array('header' => $header,'rows' => $rows));
}

/**
 * Sets up and returns Image upload form
 * @return [array]             [form]
 */
function customheaderimage_image_form($form_state) {
  $form['file'] = array(
    '#type' => 'file',
    '#title' => t('Image'),
    '#description' => t('Upload a file, allowed extensions: jpg, jpeg, png, gif'),
  );
  $form['text'] = array(
    '#type'
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/***************************************************************************
 *  Functions for finding regular page information below
 ************************************************************************
 *
 * Returns page paths in assoc array with key as the node id and path
 * as the value
 * @return [array] [nodeid=>path]
 */
function customheaderimage_get_page_paths() {
  $paths = array();
  $sql =  "SELECT source FROM url_alias";
  $results = db_query($sql)->fetchCol();

  foreach ($results as $path) {
    $nid = customheaderimage_get_nid_from_path($path);
    $paths[$nid] = $path;
  }
  return $paths;
}

/**
 * Returns the Node ID from the path
 * @param  [string] $string [ex: node/12]
 * @return [int]         [ex: 12]
 */
function customheaderimage_get_nid_from_path($string) {
  $string = explode('/', $string);
  $string = array_reverse($string);
  return $string['0'];
}

function customheaderimage_title_from_nid($nid) {
  $sql = "SELECT title FROM node WHERE nid='$nid' LIMIT 1";
  $title = db_query($sql)->fetchAssoc();
  //clean it up out of array
  $title = $title['title'];
  return $title;
}

/***************************************************************************
 * Functions for finding views information Below
 **************************************************************************
 *
 * Gets all view pages paths
 * @return [array] [all views page paths]
 */
function customheaderimage_get_view_paths() {
  $sql = "SELECT path
          FROM  menu_router
          WHERE  page_callback =  'views_page'";
  $routes = db_query($sql)->fetchCol();
  return $routes;
}

/**
 * Returns name of view from a view path. It does this by splitting it
 * up if it is a nested path and returning the element at the end
 * @param  [array] $path [specific view page path]
 * @return [string]       [Name of view assoc with that path]
 */
function customheaderimage_name_from_view_path($path) {
  $explode = explode('/', $path);
  $arr = array_reverse($explode);
  return $arr[0];
}

/**
 * Gets the title of the view page name when you pass in the name
 * @param  [string] $name [Name of view page]
 * @return [string]       [Title of that view]
 */
function customheaderimage_title_from_view_name($name) {
  $name = str_replace('-', '_', $name);
  $sql = "SELECT human_name FROM views_view WHERE name='$name' LIMIT 1";
  $title = db_query($sql)->fetchAssoc();
  $title = $title['human_name'];
  return $title;
}

 /****************************************************************************
 * Image editing form functions below
 ****************************************************************************
 *
 * Displays Image Upload form on edit page
 * @param  [type] $arg1 [description]
 * @return [array]       [form structure]
 */
function customheaderimage_edit_image_form($form, &$form_submit) {
  $html = '<h2>'.t('Upload Image').'</h2>';
  $base_path = variable_get('file_public_path', conf_path() . '/files');
  //Checking if we are on global page
  $global = preg_match('|/global$|', $_GET['q']);
  if($global) {
    $html .= '<p>'.
              t('Upload the image you want to be displayed as the header image
              globally excluding ones you set explicitly using this form. If
              you want to remove this default and use your normal css
              stylesheets click the link below')
          .'.</p>';
     // $image = customheaderimage_image_name('global');
      if($image = customheaderimage_image_name('global')) {
        $html .= '<p>'
                  .l('Click here to delete global default',
                      'admin/config/customheaderimage/delete/global')
                  .'</p>'.
                  '<p>'.t('Your current image is: ')
                    .l($image, $base_path.'/'.$image)
                  .'</p>';
      }
  }
  else {
    $html .= '<p>'.
              t('Upload the image you want to be displayed as the header image
              for this specific page using this form.')
          .'</p>';
    $page_path = customheaderimage_get_last_argument();
    if($image = customheaderimage_image_name($page_path)) {
      $html .= '<p>Your current image is: '
                  .l($image, $base_path.'/'.$image)
                .'</p>';
    }
  }

  $form['info'] = array(
    '#type' => 'markup',
    '#markup' => $html,
  );
  $form['file'] = array(
    '#type' => 'file',
    '#title' => t('Image'),
    '#description' => t('Upload a file, allowed extensions: jpg, jpeg, png, gif'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}
function customheaderimage_edit_image_form_validate($form, &$form_state) {
  $file = file_save_upload('file', array(
    'file_validate_is_image' => array(), // Validates file is really an image.
    'file_validate_extensions' => array('png gif jpg jpeg'), // Validate extensions.
  ));
  // If the file passed validation:
  if ($file) {
    // Move the file, into the Drupal file system
    if ($file = file_move($file, 'public://')) {
      // Save the file for use in the submit handler.
      $form_state['storage']['file'] = $file;
    }
    else {
      form_set_error('file', t('Failed to write the uploaded file to the site\'s file folder.'));
    }
  }
  else {
    form_set_error('file', t('No file was uploaded.'));
  }
}

function customheaderimage_edit_image_form_submit($form, &$form_state) {
  $file = $form_state['storage']['file'];
  // We are done with the file, remove it from storage.
  unset($form_state['storage']['file']);
  // Make the storage of the file permanent
  $file->status = FILE_STATUS_PERMANENT;
  // Save file status.
  file_save($file);
  $base_path = '/'.variable_get('file_public_path', conf_path() . '/files');
  $new_image_path = $base_path.'/'.$file->filename;
  $explode = explode('/', $_GET['q']);
  $page_path = array_pop($explode);
  $page_path = customheaderimage_dashtoslash($page_path);

  $global = preg_match('|/global$|', $_GET['q']);
  customheaderimage_add_rule($page_path, $new_image_path);

  // Set a response to the user.
  drupal_set_message(t('The form has been submitted and the image has been saved, filename: @filename.'
                        , array('@filename' => $file->filename)));
}

/********************************************************************
 CSS Processing functions below
 ********************************************************************
 * This function will be called on the image upload form submit
 * It will add the rule to the css when its not there or update
 * when it's different
 * @param  [string] $page_path [path to page we are checking]
 * @param  [string] $image     [path to image uploaded]
 * @return [boolean]            [True if added. False for error]
 */
  function customheaderimage_add_rule($page_path, $image) {
    // path to CSS
    $path = drupal_get_path('module', 'customheaderimage') .'/css/customheaderimage_style.css';
    $class = customheaderimage_get_class($page_path);

    $style = " #header { background-image: url('$image'); }";

    $css_class_exists = customheaderimage_class_exists($class, $path);

    if(!$css_class_exists) {
      //No image path means that the css rule does not exist yet
      if(file_put_contents($path, $class.$style, FILE_APPEND)) {
        drupal_set_message('Added CSS Style');
      }
      else {
        drupal_set_message('Error adding CSS style', 'error');
      }
    }
    else { //Rule already exists, Should we update?
      $image_path = customheaderimage_check_rule($class, $path);
      if($image_path != $image) {
        //No the same img path so Update path
        customheaderimage_update_image($class, $path, $image, $image_path);
      }
    }
  }
  function customheaderimage_get_class($page_path) {
    if($page_path != 'global') {
      if(stristr($page_path, 'node')) {
          //Set up page css styles ex: page-node-2
          $nid = customheaderimage_get_nid_from_path($page_path);
          $class = "\n.page-node-$nid";
        }
        else {
          //Set up view css styles ex: page-path-to-view
          $page_path = str_replace('/', '-', $page_path);
          $class = "\n.page-$page_path";
      }
    }
    else {
      $class = "\nbody";
    }
      return $class;
  }
  function customheaderimage_class_exists($class, $file_path) {
    $file = file_get_contents($file_path);
    if(stristr($file, $class)) {
      return TRUE;
    }
    return FALSE;
  }
  /**
   * Checks if rule already exists by checking if the class already
   * exists in the CSS file
   * @param  [string] $class [class to check]
   * @return [string]        [img path]
   */
  function customheaderimage_check_rule($class, $file_path) {
    $file = file_get_contents($file_path);
    if(stristr($file, $class)) {
      //File already has rule
      ////.page-node-1 #header\{ background-image: url\('([a-z/_0-9.]+)
      $string = preg_match(
                  "|$class #header \{ background-image: url\('([a-zA-Z/_0-9.]+)|"
                  , $file, $matches);
      $image_path = $matches[1];
      return $image_path;
    }
    return FALSE;
  }
  function customheaderimage_update_image($class, $file_path, $new_image, $old_image) {
    $file = file_get_contents($file_path);
    $new_rule =  $class . " #header { background-image: url('$new_image'); }";
    $old_rule =  $class . " #header { background-image: url('$old_image'); }";

    $file = str_replace($old_rule, $new_rule, $file);
    if(file_put_contents($file_path, $file)) {
      drupal_set_message('Updated header image.');
      return TRUE;
    }
    return FALSE;
  }

  function customheaderimage_remove_rule($page_path) {
    $path = drupal_get_path('module', 'customheaderimage') .'/css/customheaderimage_style.css';
    $class = customheaderimage_get_class($page_path);

    if(customheaderimage_class_exists($class, $path)) {
      $image_path = customheaderimage_check_rule($class, $path);
      $file = file_get_contents($path);
      $rule =  $class . " #header { background-image: url('$image_path'); }";

      $file = str_replace($rule, '', $file);
      if(file_put_contents($path, $file)) {
        return TRUE;
      }
    }
    return FALSE;
  }
  function customheaderimage_image_name($page_path) {
    $path = drupal_get_path('module', 'customheaderimage') .'/css/customheaderimage_style.css';
    $class = customheaderimage_get_class($page_path);
    $image_path = customheaderimage_check_rule($class, $path);
    $explode = explode('/', $image_path);
    return array_pop($explode);
  }

/********************************************************************
Delete rule functinos below
 ********************************************************************
 *
 * Menu callback; get confirm form for deleting search history.
 */
function customheaderimage_delete() {
  return drupal_get_form('customheaderimage_delete_confirm');
}


/**
 * Build a confirm form for deletion of search history.
 */
function customheaderimage_delete_confirm($form, &$form_state) {
  $caption = '<p>' . t('This action cannot be undone.') . '</p>';
  $heading = t('Are you sure you want to clear the header image for this page?');
  $cancel_path = 'admin/config/customheaderimage';
  $yes = t('Delete Image');
  $no = t('Cancel');
  return confirm_form($form, $heading, $cancel_path, $caption, $yes, $no);
}

/**
 * Submit function for the confirm deletion form.
 */
function customheaderimage_delete_confirm_submit($form, &$form_state) {
  //variable_del('input_search_history');
  customheaderimage_remove_rule(customheaderimage_get_last_argument());
  drupal_set_message('This path specific header image has been cleared.');
}


/********************************************************************
 Helper functions below
 ********************************************************************/
function customheaderimage_slashtodash($string) {
  return str_replace('/', '--', $string);
}
function customheaderimage_dashtoslash($string) {
  return str_replace('--', '/', $string);
}
function customheaderimage_get_last_argument() {
  $explode = explode('/', $_GET['q']);
  return customheaderimage_dashtoslash(array_pop($explode));
}
