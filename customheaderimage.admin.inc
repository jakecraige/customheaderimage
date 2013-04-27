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
    'operations'    => t('Operations'),
  );
  $rows = array();
  foreach ($paths as $title=>$path) {
    $rows[] = array(
                'title' => l($title, $path),
                'path' => $path,
                'operations' => l('edit', 'admin/config/customheaderimage/edit/'.$title),
    );
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
  $arr = array_reverse(explode('/', $path));
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
 * Image editing functions below
 ****************************************************************************
 *
 * Displays Image Upload form on edit page
 * @param  [type] $arg1 [description]
 * @return [array]       [form structure]
 */
function customheaderimage_edit_image_form($form, &$form_submit) {
  $html = '<h2>'.t('Upload Image for @arg1', array( '@arg1' => 'test')).'</h2>';
  $html .= '<p>'.
              t('Upload the image you want to be displayed as the header image
              using this form.')
          .'</p>';
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

  // Set a response to the user.
  drupal_set_message(t('The form has been submitted and the image has been saved, filename: @filename.', array('@filename' => $file->filename)));
}
