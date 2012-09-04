/*
 *
 * Copyright (C) 2012 eZuce Inc.
 *
 * $
 */
package org.sipfoundry.sipxconfig.sipxsbc;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxsbcSettings extends PersistableSettings implements DeployConfigOnEdit {

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Arrays.asList((Feature) Sipxsbc.SIPXSBC_FEATURE, (Feature) FirewallManager.FEATURE);
    }

    @Override
    public String getBeanId() {
        return "sipxsbcSettings";
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxsbc/sipxsbc.xml");
    }

    public int getBaseTransportPort() {
        return (Integer) getSettingTypedValue("sipxsbc/tcp-port-base");
    }

    public int getMaxTransportPort() {
        return (Integer) getSettingTypedValue("sipxsbc/tcp-port-max");
    }

    public int getTcpListenerPort() {
        return (Integer) getSettingTypedValue("sipxsbc/listener-port");
    }

}