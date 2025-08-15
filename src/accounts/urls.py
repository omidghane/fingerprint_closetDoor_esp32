from django.urls import path, include
from . import views

app_name = 'accounts'

urlpatterns = [
    path('login/', views.login_view, name='login'),
    path('register/', views.register_view, name='register'),
    path('logout/', views.logout_view, name='logout'),
    path('verify/<str:phone>', views.verify, name='verify'),
    path('login/phone', views.login_phone_view, name='login-phone'),
    path('api/v1/', include('accounts.api.v1.urls', namespace='accounts-api')),


    
]
