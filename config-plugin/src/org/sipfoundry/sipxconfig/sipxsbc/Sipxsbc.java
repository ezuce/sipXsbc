/*
 *
 * Copyright (C) 2012 eZuce Inc.
 *
 * $
 */
package org.sipfoundry.sipxconfig.sipxsbc;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface Sipxsbc {
    public static final LocationFeature SIPXSBC_FEATURE = new LocationFeature("sipxsbc");
    public static final AddressType SIPXSBC_LISTENER_ADDRESS = new AddressType("sbcTcp");
    public static final AddressType SIPXSBC_TRANSPORT_ADDRESS = new AddressType("sbcTcpTransport");

    SipxsbcSettings getSettings();

    void saveSettings(SipxsbcSettings settings);

}