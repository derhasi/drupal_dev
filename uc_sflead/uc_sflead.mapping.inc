<?php
// $Id$

/**
 * Mapping callbacks.
 */


/**
 * Callback for setting mappings.
 */
function _uc_sflead_mapping_callback_setting($mapping, $order) {
  return _uc_sflead_mapping_callback_fixed($mapping, $order);
}

/**
 * Callback for setting mappings.
 */
function _uc_sflead_mapping_callback_fixed($mapping, $order) {
  return $mapping->value;
}

/**
 * Callback for setting mappings.
 */
function _uc_sflead_mapping_callback_variable($mapping, $order) {
  return $mapping->value;
}

/**
 * Callback for setting mappings.
 */
function _uc_sflead_mapping_callback_token($mapping, $order) {
  // Fallback for non-existant token.
  if (!module_exists('token')) {
    return _uc_sflead_mapping_callback_fixed($mapping, $order);
  }

  $output = token_replace($mapping->value, 'global');
  return token_replace($output, 'order', $order);
}