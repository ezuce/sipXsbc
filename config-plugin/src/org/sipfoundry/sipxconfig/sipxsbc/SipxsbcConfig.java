/*
 *
 * Copyright (C) 2012 eZuce Inc.
 *
 * $
 */
package org.sipfoundry.sipxconfig.sipxsbc;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.springframework.beans.factory.annotation.Required;

public class SipxsbcConfig implements ConfigProvider {

    private Sipxsbc m_sbc;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(Sipxsbc.SIPXSBC_FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        FeatureManager featureManager = manager.getFeatureManager();
        SipxsbcSettings settings = m_sbc.getSettings();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = featureManager.isFeatureEnabled(Sipxsbc.SIPXSBC_FEATURE, location);

            ConfigUtils.enableCfengineClass(dir, "sipxsbc.cfdat", enabled, "sipxsbc");
            if (!enabled) {
                continue;
            }
            File f = new File(dir, "sipxsbc.ini.part");
            Writer wtr = new FileWriter(f);
            try {
                KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(wtr);
                config.write("listener-address", location.getAddress());
                config.write("external-address", location.getPublicAddress());
                config.writeSettings(settings.getSettings());
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    @Required
    public void setSipxsbc(Sipxsbc sbc) {
        m_sbc = sbc;
    }

}