some_db = (
    <database>
        <table name="foo">
            <column name="id" type="serial"/>
            <column name="name" type="string"/>
            <column name="data" type="json"/>
        </table>
    </database>
)

//
// MVP

something = (
    <aws-ec2-instance name="foo" type="ec2.nano" ami="blabla" security-group="aaa">
        <ebs-volume-attachment name="os" size="8GB"  device="/dev/sda"/>
        <ebs-volume name="data" size="20GB" device="/dev/sdb"/>

        {some_os()}
        {/* aws-ec2-instance, together with do-droplet provides
            context to underlying <os> instance, i.e
                hostIp
        */}
    </aws-ec2-instance>
)

// when rendering,
//   find all instances with Tag key="react-any-key" $name"
//   these are our "actual status
//   for these we have "ip", instance-id
//   volumes

some_os = function(props) {
    return (
        <os {props...}>
            <user name="foo" groups="admin,sudoers"/>
            <apt>
                <package name="vim"/>
                <package name="openssh-server"/>
                <package name="redis-server"/>
                <package name="nginx"/>
                {/*
                    or even
                    react-whatever allows "type override", so all
                    elements under apt, have custom handler i.e tag name is package name
                */}

                <vim/>
                <redis-server/>
            </apt>
            <file target="/etc/ssl/mycompany.com.crt" source="@mycompany.com.crt"/>
            <file target="/etc/ssl/mycompany.com.key" source="@mycompany.com.key"/>

            <systemd-service>
            </systemd-service>
            <lvm-vg name="data">
                <pv dev="/dev/xvda1"/>
                <lv name="redis" size="10G" fs="ext4"/>
                <lv name="backups" size="10G" fs="ext4"/>
            </lvm-vg>
            <mount dev="/dev/mapper/data--vg-redis" target="/var/lib/redis"/>
            <mount dev="/dev/mapper/data--vg-backups" target="/var/lib/backups"/>

                {/* example
                  mount point, when created automagically
                  and target exists and is non-empty,
                     automagically creates ${target}.original, mounts and moves contents to newly mounted target
                */}
        </os>
    )
}

// FIRST IDEA

UserSchema = ReactAny.createSchema({
    key: 'name',
    propTypes: {
        name: PropTypes.string.isRequired,
        uid: PropTypes.number,
        groups: PropTypes.string,
    },
    // render: is implicit and defaults to return "react equivalent" () => ( this.props.children )
})

LinuxUserImpl = ReactAny.createRenderer({
    list: function() {
        return this.context.shell("getent passwd")
            .then((stdout) => {
                // parse getent output
                // username:nothing:id:
                return [];
            })
    },
    get: function(props) {
        return this.context.shell("getent passwd " + props.name)
            .then((stdout) => {
                // parse getent output
                // username:nothing:id:
                return { name: props.name /* ... */ };
            })
    },
    create: function(props) {
        return this.context.shell("useradd " + props.name);
    },
    delete: function(props) {
        return this.context.shell("userdel " + props.name);
    },
    update: function(props) {
        // now slightly different logic
        return this.context.shell("usermod " + props.name);
    }
});

var ReactOSSchema = {
    types: {
        user: UserSchema,
        apt: APTSchema,
        file: PlainFileSchema,
        'lvm-vg': ReactOS.contextProviderSchema((props) => ({ 'lvm-vg': props.name }))
    }
}

// second ALTERNATIVE for component definition

UserSchema = ReactAny.createSchema({
    key: 'name',
    propTypes: {
        name: PropTypes.string.isRequired,
        uid: PropTypes.number,
        groups: PropTypes.string,
    },
    // render: is implicit and defaults to return "react equivalent" () => ( this.props.children )
    create: function(props) {
        return this.context.shell("useradd " + props.name);
    },
    delete: function(props) {
        return this.context.shell("userdel " + props.name);
    },
    update: function(props) {
        // now slightly different logic
        return this.context.shell("usermod " + props.name);
    }
})

ReactOS = ReactAny.createRender(ReactOSSchema, {
    os: ReactAny.SimpleContainerSchema,
    user: LinuxUserImpl
})

ReactOS.render(some_os, {
    shell: SSHShell("root@server.mycompany.com")
});
/*
    this will render a tree of
    <os>
        (...)
    </os>
        and for each children
 */

