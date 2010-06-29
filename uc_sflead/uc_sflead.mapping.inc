<?php
// $Id$

/**
 * Mapping callbacks.
 */


/**
 * Callback for setting values.
 */
function _uc_sflead_mapping_callback_setting($mapping, $order) {
  return _uc_sflead_mapping_callback_fixed($mapping, $order);
}

/**
 * Callback for fixed values.
 */
function _uc_sflead_mapping_callback_fixed($mapping, $order) {
  return $mapping->value;
}

/**
 * Callback for variable values.
 */
function _uc_sflead_mapping_callback_variable($mapping, $order) {
  $keys = explode(UC_SFLEAD_SEPARATOR_ARRAY, $mapping->value);
  return _uc_sflead_mapping_callback_variable_helper($keys, (array) $order);
}


/**
 * Helper function for variable values
 */
function _uc_sflead_mapping_callback_variable_helper($keys, $arr) {
  $count = count($keys);
  $current_key = array_shift($keys);

  // Pass NULL for invalid combinations.
  if (!$count || !is_array($arr) || empty($arr) || !isset($arr[$current_key])) {
    return NULL;
  }

  $new = $arr[$current_key];
  // Has to be an array.
  if (is_object($new)) {
    $new = (array) $new;
  }

  // If more keys are present, new arr has to be an array
  if (count($keys) && is_array($new)) {
    return _uc_sflead_mapping_callback_variable_helper($keys, $new);
  }
  elseif (!count($keys) && !is_array($new)) {
    return $new;
  }

}

/**
 * Callback for token values.
 */
function _uc_sflead_mapping_callback_token($mapping, $order) {
  // Fallback for non-existant token.
  if (!module_exists('token')) {
    return _uc_sflead_mapping_callback_fixed($mapping, $order);
  }

  $account = user_load(array('uid' => $order->uid));

  $output = token_replace($mapping->value, 'global');
  $output = token_replace($output, 'order', $order);
  return token_replace($output, 'user', $account);
}