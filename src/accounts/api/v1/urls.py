from django.urls import path
from accounts.views import *

app_name = 'accounts-api'

urlpatterns = [
    path('jwt/create/', CustomTokenObtainPairView.as_view(), name='token_create'),
    path('jwt/refresh/', CustomTokenRefreshView.as_view(), name='token_refresh'),
    path('user/profile/', ProfileViewSet.as_view({'get': 'list'}), name='user-profile'),
    path('user/profile/<int:pk>', ProfileViewSet.as_view({'get': 'retrieve'}), name='user-profile-detail'),
    
    path('user/doorinsert/', UserFingerDoor.as_view({'post': 'finger_insert'}), name='doorinsert'),
    path('user/doorinsert/ack/', UserFingerDoor.as_view({'post': 'finger_insert_ack'}), name='doorinsert-ack'),
    
    path('user/fingerdoor/', UserFingerDoor.as_view({'post': 'singledoor_unlock'}), name='doorlog'),
    path('user/fingerdoor/ack/', UserFingerDoor.as_view({'post': 'singledoor_unlock_ack'}), name='doorlog-ack'),
    path('user/fingerclosetdoor/ack/', UserFingerDoor.as_view({'post': 'closetdoor_unlock_ack'}), name='closetdoorlog-ack'),
    
    path('user/doordelete/', UserFingerDoor.as_view({'post': 'delete_closet_door'}), name='doordelete'),
    path('user/fingerlog/', UserFingerDoor.as_view({'post': 'log_insert'}), name='user-finger-log'),

    path('user/moistureread/', PlantWateringClass.as_view({'post': 'moisture_read'}), name='moisture_read'),
    path('user/tempshow/', TempratureClass.as_view({'post': 'temprature_read'}), name='temp_show'),

    path('user-finger-form/', user_finger_form, name='user_finger_form'),
    path('user-closet-form/', user_closet_form, name='user_closet_form'),
]
