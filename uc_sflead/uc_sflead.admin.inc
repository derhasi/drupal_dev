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
    '#value' => l(t('Add new mapping'), 'admin/store/settings/orders/edit/sflead/add'),
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

  $form['debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Debug mode'),
    '#default_value' => variable_get('uc_sflead_debug_mode', FALSE),
    '#description' => t('Enabling debug mode will write response messages to watchdog and show response objects on order updates (if devel.module) is enabled.'),
  );

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
        l(t('edit'), "admin/store/settings/orders/edit/sflead/$item->sfkey"),
        l(t('delete'), "admin/store/settings/orders/edit/sflead_delete/$item->sfkey"),
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
    '#title' => t('Mapping type'),
    '#default_value' => (!$mapping) ? FALSE : $mapping->mapping_type,
    '#required' => TRUE,
    '#options' => array(
      'fixed' => t('Fixed value'),
      'variable' => t('Variable value'),
    ),
    '#description' => t('See help box below for more information on the different mapping types.'),
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

  // Help text is showing dependent on selection.
  if (module_exists('ctools')) {
    ctools_include('dependent');
  }

  $form['help'] = array(
    '#type' => 'fieldset',
    '#title' => t('Help'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['help']['fixed'] = array(
    '#input' => TRUE,
    '#type' => 'item',
    '#title' => t('Help for %selection', array('%selection' => t('Fixed value'))),
    '#value' => t('The given value will be passed to salesforce without modifications.'),
    '#dependency' => array('edit-mapping-type' => array('fixed')),
    '#process' => array('ctools_dependent_process'),
  );

  $form['help']['variable'] = array(
    '#input' => TRUE,
    '#type' => 'item',
    '#title' => t('Help for %selection', array('%selection' => t('Variable value'))),
    '#value' => t('The value is an indicator for the value in the $order object. Levels of object and array are separated by "!arraysep".'.
                  '<br> Examples:'.
                  '<ul><li>$order->billing_first_name : <code>billing_first_name</code></li>'.
                  "<li>\$order->data['profile']['profile_xy'] : <code>data!ARRSEPprofile!ARRSEPprofile_xy</code></li>".
                  "<li>\$order->extra_fields['ucxf_1_billing'] : <code>extra_fields!ARRSEPucxf_1_billing</code></li></ul>.",
                  array('!ARRSEP' => UC_SFLEAD_SEPARATOR_ARRAY)),
    '#dependency' => array('edit-mapping-type' => array('variable')),
    '#process' => array('ctools_dependent_process'),
  );

  $form['help']['token'] = array(
    '#input' => TRUE,
    '#type' => 'item',
    '#title' => t('Help for %selection', array('%selection' => t('Token'))),
    '#value' => t('The given value will be replaced with global and order tokens. !token has to be enabled. !token_replacement',
                  array(
                    '!token' => l('Token', 'http://drupal.org/project/token'),
                    '!token_replacement' => theme('token_help', array('global', 'order', 'user')),
                  )),
    '#dependency' => array('edit-mapping-type' => array('token')),
    '#process' => array('ctools_dependent_process'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#submit' => array('uc_sflead_settings_mapping_submit'),
    '#value' => t('Submit'),
  );
  $form['cancel'] = array(
    '#value' => l(t('Cancel'), 'admin/store/settings/orders/edit/sflead'),
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
              "admin/store/settings/orders/edit/sflead/$sfkey/edit"
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
  $form_state['redirect'] = 'admin/store/settings/orders/edit/sflead';
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

  // Store debug value
  variable_set('uc_sflead_debug_mode', $form_state['values']['debug']);

  $form_state['redirect'] = 'admin/store/settings/orders/edit/sflead';
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
    'admin/store/settings/orders/edit/sflead',
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
  $form_state['redirect'] = 'admin/store/settings/orders/edit/sflead';
}
