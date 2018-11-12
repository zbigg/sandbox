// router2


function UserPageData(props) {
    return {
        user: '/tiqapi/users/' + props.id
    }
}

const UserPage = withData(UserPageData, ({id}) => (
    <RoutedTabs default="general">
        <Tab name="general" label="foo" icon="bar">
            /app/user/SOME_CONCRETED_ID/general
            <AccessTabContent/>
        </Tab>
        <Tab name="personal" label="foo" icon="bar">
            /app/user/SOME_CONCRETED_ID/personal
            <AccessTabContent/>
        </Tab>
    </RoutedTabs>
)

const App = () => (
    <RouteRoot path="/app">
        <RoutedNavBar>
            <NavPage name="settings" label="foo" icon="bar" component={SettingsPage}>
            <NavPage name="users" label="foo" icon="bar" component={UsersList}/>

        </RoutedTabs>
        <DynamicRoute name="user/:id" label="foo" icon="bar" component={UserPage}/>
            // UserTabContent will have props.id
            // and /app/user/SOME_CONCRETED_ID in context.current_route
    </RouteRoot>
)

-->
[ title  --- hamburger(settings, users) ]
[     PAGE                              ]