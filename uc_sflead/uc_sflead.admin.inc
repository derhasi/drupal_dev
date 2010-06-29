<?php
// $Id$

/**
 * Admin forms for uc_sflead.
 */

/**
 * Basic settings form and overview of mappings.
 */
function uc_sflead_settings_form(&$form_state) {

  // Overview
  $form['mapping_list'] = array(
    '#value' => uc_sflead_mapping_overview(),
  );

  $form['add'] = array(
    '#value' => l(t('Add new mapping'), 'admin/store/settings/checkout/edit/sflead/add'),
  );

  // Settings
  $form['setting'] = array(
    '#type' => 'fieldset',
    '#title' => t('Settings'),
    '#collapsible' => TRUE,
    '#tree' => TRUE,
  );
  foreach (_uc_sflead_mapping_settings() as $key => $description) {
    $form['setting'][$key] = array(
      '#type' => 'textfield',
      '#title' => $key,
      '#default_value' => uc_sflead_mapping_info('setting', $key),
      '#description' => $description,
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#submit' => array('uc_sflead_settings_setting_submit'),
  );

  return $form;
}

/**
 * Render overview list.
 */
function uc_sflead_mapping_overview() {
  $items = uc_sflead_mapping_info('raw');

  $headers = array(
    t('key'),
    t('mapping type'),
    t('value'),
    t('edit'),
    t('delete'),
  );

  ksort($items);
  $rows = array();
  foreach ($items as $item) {
    if ($item->mapping_type != 'setting') {
      $row = array(
        check_plain($item->sfkey),
        check_plain($item->mapping_type),
        check_plain($item->value),
        l(t('edit'), "admin/store/settings/checkout/edit/sflead/$item->sfkey"),
        l(t('delete'), "admin/store/settings/checkout/edit/sflead_delete/$item->sfkey"),
      );
      $rows[] = $row;
    }
  }

  // Show warning for no mappings.
  if (!count($rows)) {
    return '<div class="warning">'. t('There are now mappings set!') .'</div>';
  }

  // render as table and return
  return theme('table', $headers, $rows);

}


/**
 * Form callback to add new field mapping.
 */
function uc_sflead_settings_add_mapping(&$form_state) {
  $form = array();
  _uc_sflead_mapping_form_helper($form, FALSE);
  $form['new'] = array(
    '#type' => 'value',
    '#value' => TRUE,
  );
  return $form;
}
/**
 * Form callback to edit existing field mapping.
 */
function uc_sflead_settings_edit_mapping(&$form_state, $mapping_info, $allow_setting = FALSE) {
  $form = array();

  if (!is_object($mapping_info) || ($mapping->mapping_type == 'setting' && !$allow_setting)) {
    return drupal_not_found();
  }
  _uc_sflead_mapping_form_helper($form, $mapping_info);
  return $form;
}

/**
 * Helper function for mapping add and edit
 */
function _uc_sflead_mapping_form_helper(&$form, $mapping) {

  // Mapping key for new or existing item.
  if (!$mapping) {
    $form['sfkey'] = array(
      '#type' => 'textfield',
      '#title' => t('Salesforce form key'),
      '#default_value' => '',
      '#required' => TRUE,
    );
  }
  else {
    $form['sfkey'] = array(
      '#type' => 'value',
      '#value' => $mapping->sfkey,
    );
    $form['sfkey_label'] = array(
      '#value' => check_plain($mapping->sfkey),
      '#prefix' => '<strong>'. t('Salesforce form key') .':</strong><br/>',
    );
  }

  // Mapping type
  $form['mapping_type'] = array(
    '#type' => 'select',
    '#title' => t('Select'),
    '#default_value' => (!$mapping) ? FALSE : $mapping->mapping_type,
    '#required' => TRUE,
    '#options' => array(
      'fixed' => t('Fixed value'),
      'variable' => t('Variable value'),
    ),
    // @TODO: advanced help integration
  );
  if (module_exists('token')) {
    $form['mapping_type']['#options']['token'] = t('Token');
  }

  $form['value'] = array(
    '#type' => 'textfield',
    '#title' => t('Value'),
    '#description' => t('Fixed value or specification for variable or token'),
    '#default_value' => (!$mapping) ? FALSE : $mapping->value,
  );

  $form['#validate'] = array(
    'uc_sflead_settings_mapping_validate',
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#submit' => array('uc_sflead_settings_mapping_submit'),
    '#value' => t('Submit'),
  );
  $form['cancel'] = array(
    '#value' => l(t('Cancel'), 'admin/store/settings/checkout/edit/sflead'),
  );
}

/**
 * Validate new mapping setting.
 */
function uc_sflead_settings_mapping_validate($form, &$form_state){
  // Check for existing sfkey
  if ($form_state['values']['new']) {
    $sfkey = $form_state['values']['sfkey'];
    $ex = _uc_sflead_mapping_exists($sfkey);
    // Key is not allowd
    if ($ex == 2) {
      form_set_error('sfkey', t('The key %key is allreadey reserved for basic settings. please choose another one.', array('%key' => $sfkey)));
    }
    // Key allready exists.
    elseif ($ex) {
      form_set_error('sfkey',
        t('There is allready a mapping for key %key. Please !edit the existing one or use another key.',
          array(
            '%key' => $sfkey,
            '!edit' => l(
              t('edit'),
              "admin/store/settings/checkout/edit/sflead/$sfkey/edit"
            )
          )
        )
      );
    }
  }
}

/**
 * Store new or updated mapping.
 */
function uc_sflead_settings_mapping_submit($form, &$form_state) {
  $new = $form_state['values']['new'];
  $write = (object) $form_state['values'];
  if ($new) {
    drupal_write_record('uc_sflead_mapping', $write);
  }
  else {
    drupal_write_record('uc_sflead_mapping', $write, array('sfkey'));
  }
  $form_state['redirect'] = 'admin/store/settings/checkout/edit/sflead';
}

/**
 * Store basic settings.
 */
function uc_sflead_settings_setting_submit($form, &$form_state) {
  // Current info to retrieve new/update status.
  $info = uc_sflead_mapping_info('setting');

  foreach ($form_state['values']['setting'] as $key => $value) {
    $obj = (object) array(
      'sfkey' => $key,
      'mapping_type' => 'setting',
      'value' => $value,
    );

    if ($info[$key]) {
      drupal_write_record('uc_sflead_mapping', $obj, array('sfkey'));
    }
    else {
      drupal_write_record('uc_sflead_mapping', $obj);
    }
  }
  $form_state['redirect'] = 'admin/store/settings/checkout/edit/sflead';
}


/**
 * Delete mapping.
 */
function uc_sflead_mapping_delete_confirm(&$form_state, $mapping) {
  if (!is_object($mapping) || $mapping->mapping_type == 'setting') {
    return drupal_not_found();
  }

  // Key for mapping to delete.
  $form['key'] = array(
    '#type' => 'value',
    '#value' => $mapping->sfkey,
  );

  return confirm_form($form,
    t('Are you sure you want to delete mapping %key of type %type?', array('%key' => $mapping->sfkey, '%type' => $mapping->mapping_type)),
    'admin/store/settings/checkout/edit/sflead',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submission after delete was confirmed.
 */
function uc_sflead_mapping_delete_confirm_submit($form, &$form_state) {
  $key = $form_state['values']['key'];
  uc_sflead_mapping_delete($key);
  uc_sflead_mapping_info(NULL, NULL, TRUE);
  $form_state['redirect'] = 'admin/store/settings/checkout/edit/sflead';
}
