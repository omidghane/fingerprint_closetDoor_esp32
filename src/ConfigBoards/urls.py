from django.urls import path, include
from . import views

app_name = 'ConfigBoards'

urlpatterns = [
    path('', views.get_config_website, name='config'),
    path('wificonfig', views.get_wificonfig, name='wificonfig'),
    path('networkconfig', views.get_networkconfig, name='networkconfig'),    
    path('authconfig', views.get_authconfig, name='authconfig'),
    path('command', views.get_command, name='command')

    
]
