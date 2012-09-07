/*
 *
 * Copyright (C) 2012 eZuce Inc.
 *
 * $
 */
package org.sipfoundry.sipxconfig.sipxsbc;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.firewall.FirewallRule;
import org.sipfoundry.sipxconfig.redis.Redis;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class SipxsbcImpl implements Sipxsbc, FeatureProvider, AddressProvider, ProcessProvider, FirewallProvider {
    private static final List<AddressType> ADDRESSES = Arrays.asList(SIPXSBC_LISTENER_ADDRESS, SIPXSBC_TRANSPORT_ADDRESS);
    private BeanWithSettingsDao<SipxsbcSettings> m_settingsDao;

    @Override
    public SipxsbcSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(SipxsbcSettings settings) {
        m_settingsDao.upsert(settings);
    }

    public void setSettingsDao(BeanWithSettingsDao<SipxsbcSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiredOnSameHost(SIPXSBC_FEATURE, Redis.FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        // TODO Auto-generated method stub
        
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return DefaultFirewallRule.rules(
                Arrays.asList(SIPXSBC_LISTENER_ADDRESS, SIPXSBC_TRANSPORT_ADDRESS),
                FirewallRule.SystemId.PUBLIC, true);
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        FeatureManager featureManager = manager.getFeatureManager();
        if (!featureManager.isFeatureEnabled(SIPXSBC_FEATURE, location)) {
            return null;
        }
        ProcessDefinition def = ProcessDefinition.sipx("sipxsbc");
        return Collections.singleton(def);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(SIPXSBC_FEATURE);
        SipxsbcSettings settings = getSettings();

        if (type.equals(SIPXSBC_LISTENER_ADDRESS)) {
            return Location.toAddresses(type, locations, settings.getTcpListenerPort());
        }

        List<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            Address a = new Address(type, location.getAddress(), settings.getBaseTransportPort());
            a.setEndPort(settings.getMaxTransportPort());
            addresses.add(a);
        }
        return addresses;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(SIPXSBC_FEATURE);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(SIPXSBC_FEATURE);
        }
        
    }

}